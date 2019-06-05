#include "Spacecraft.h"

void aocsSoftware(vec3 position, vec3 velocity, mat3 attitude, vec3 angularVel, double julian, vec3& torqueOut, vec3& thrustOut){
    torqueOut = vec3{0.0,0.0,0.0};
    thrustOut = vec3{0.0,0.0,0.0};
}


spacecraftState Spacecraft::dynamicsEquation(double time, spacecraftState cs){
    spacecraftState ds;
    ds.JDTDB = 1; // dt/dt
    ds.position = cs.velocity; // dx/dt
    vec3 aocsTorque, aocsThrust;
    aocsSoftware(cs.position, cs.velocity, cs.attitude, cs.angularVel, time, aocsTorque, aocsThrust);
    ds.angularVel = transpose(cs.inertialMat)*aocsThrust;// dw/dt
    ds.mass = len(aocsThrust) / (this->ISP * 9.81);// dm/dt
    vec3 omegaB = cs.attitude*cs.angularVel;
     ds.attitude = (cs.attitude* -1.0) * crossMat(cs.angularVel);
    ds.inertialMat = zeros3();// dJ/dt
    vec3 acceleration = this->motionEqnBarycentric(cs.JDTDB, cs.position);
    //acceleration += qrot(aocsThrust, cs.attitude);
    ds.velocity = acceleration;// dv/dt
    return ds;
}

vec3 Spacecraft::motionEqnBarycentric(double time, vec3 position) {
    vec3 acceleration = vec3{ 0.0,0.0,0.0 };
    for (auto const& [key, planet]: solarSystem->Planets) {
        //cout << planet->name + ": " << (planet->calculateAbsoluteAccelerationFromPosition(time, position)) << endl;
        acceleration += planet->calculateAbsoluteAccelerationFromPosition(time, position);
        for (int j = 0; j < planet->children.size(); ++j) {
            acceleration += planet->children[j]->calculateAbsoluteAccelerationFromPosition(time, position);
        }
    }
    return acceleration;
}
Spacecraft::Spacecraft(string name, double startTime, vec3 startPos, vec3 startVel, string craftFile = "craft") {
    this->name = name;
    this->timeOfYeet = startTime;
    texHandler->loadTexture(name, true, false);
    this->bodyMesh = new SceneObject(name, startPos);
    this->bodyMesh->setScale(5.0);
    
    this->bodyMat = new Material("DS_1", name);
    this->bodyMat->addObj(this->bodyMesh);
    this->position = startPos;
    this->velocity = startVel;
    this->solver = [this](double _a,spacecraftState _b){return this->dynamicsEquation(_a,_b);};

    float sunStr = 5.0;
    float sunLin = 5e-9;
    float sunQuad = 0.0;
    if (bodyMat != NULL) {
        this->bodyMat->matShader->setUniformV3("sun.color", vec3f{ 1.0f,1.0f,0.95f });
        this->bodyMat->matShader->setUniformV3("sun.position", vec3{ 0.0,0.0,0.0 });
        this->bodyMat->matShader->setUniform1f("sun.attn_str", sunStr);
        this->bodyMat->matShader->setUniform1f("sun.attn_lin", sunLin);
        this->bodyMat->matShader->setUniform1f("sun.attn_quad", sunQuad);
    }

}

void Spacecraft::addPos(vec3 pos) {
    this->position += pos;
    this->bodyMesh->addPos(pos);
}

void Spacecraft::addVel(vec3 vel) {
    this->velocity += vel;
}

void Spacecraft::setPosition(vec3 pos) {
    this->position = pos;
    if (bodyMesh != NULL)
        this->bodyMesh->setPos(pos);
}

void Spacecraft::setVelocity(vec3 vel) {
    this->velocity = vel;
}
void Spacecraft::setAttitude(mat3 q){
    this->attitude = q;
    if (bodyMesh!= NULL)
        this->bodyMesh->setAttitude(mat3expand(q));
}
	
double Spacecraft::getMass(){
    return this->mass;
}
double Spacecraft::getYeet(){
    return this->timeOfYeet;
}
double Spacecraft::getYote(){
    return this->timeOfYote;
}

	
// Calls the ODE solver and saves the returned ephemeris data.
void Spacecraft::propagate(double initTime, double deltaTime, double tolerance) {
    this->timeOfYeet = initTime;
    this->timeOfYote = initTime + deltaTime;
    printf("Beginning spacecraft propagation. Working...");
    spacecraftState odeInput;
    odeInput.position = this->position; cout << position << endl;
    odeInput.velocity = 86400.0*this->velocity; cout << velocity << endl;
    odeInput.JDTDB = initTime;
    odeInput.attitude = {0.0,0.0,0.0,1.0};
    odeInput.angularVel = 86400.0*vec3{0.01,0.0,0.0};
    odeInput.inertialMat = eye3();
    odeInput.mass = 1000.0;
    //vec3_rkf45_sc(odeInput, deltaTime, tolerance, solver, &propagationData);
    printf(" done!\n");
    
    /*  MOVE TO SAVE TO FILE FUNCTION LATER
    std::ofstream craftEpm("craft.epm");
    if (craftEpm.is_open()) {
        craftEpm.precision(15);
        for (int i = 0; i < propagationData.size(); ++i) {
            craftEpm << propagationData[i].JDTDB << ",";
            craftEpm << propagationData[i].position << ",";
            craftEpm << propagationData[i].velocity << "\n";
        }
        craftEpm.close();
    }
    */
    this->shouldDraw = true;
}

// Loads saved craft ephemeris data instead of propagating it
void Spacecraft::loadEpm(string filename) {
    /*
    string path = "Ephemeris/" + filename;
    FILE* file;

    #ifdef __APPLE__
    file = fopen(path.c_str(), "r");
    bool err = (file == NULL);
    #elif defined _WIN32 || _WIN64
    errno_t err = fopen_s(&file, path.c_str(), "r");
    #endif

    if (err) {
        cout << "Could not open file: " << path << endl;
        return;
    }
    cout << "Beginning ephemeris read of " << path << endl;
    int res=1;
    while (res!=EOF) {
        double julian;
        vec3 pos;
        vec3 vel;
        res = fscanf(file, "%lg,%lg,%lg,%lg,%lg,%lg,%lg\n", &julian, &pos[0], &pos[1], &pos[2], &vel[0], &vel[1], &vel[2]);
        spacecraftState newState = { julian,pos,vel };
        propagationData.push_back(newState);
    }
    fclose(file);
    */
}

//Generates lines of the spacecraft movement in inertial frame
void Spacecraft::genOrbitLines() {
    lineMat = new Material("line", "NULL");
    lineMat->matShader->use();
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);


    std::vector<double> lineData;

    for (int i = 0; i < propagationData.size(); ++i) {
        lineData.push_back(propagationData[i].position[0]);
        lineData.push_back(propagationData[i].position[1]);
        lineData.push_back(propagationData[i].position[2]);
        lineData.push_back(0.8);
        lineData.push_back(0.8);
        lineData.push_back(0.4);
    }

    lineDataSize = lineData.size() / 6;


    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, lineDataSize * 6 * sizeof(double), lineData.data(), GL_STATIC_DRAW);
    glVertexAttribLPointer(0, 3, GL_DOUBLE, sizeof(double) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribLPointer(1, 3, GL_DOUBLE, sizeof(double) * 6, (void*)(3 * sizeof(GL_DOUBLE)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

//Generates the lines of SV movement with respect to a non-rotating body frame
void Spacecraft::genOrbitLines(PlanetaryBody* parent) {
    parentBody = parent;
    lineMat = new Material("line", "NULL");
    lineMat->matShader->use();
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);


    std::vector<double> lineData;
    statevec newState;
    statevec newStateP;
    for (int i = 0; i < propagationData.size(); ++i) {
        newState = parentBody->getStateAtTime(propagationData[i].JDTDB);
        if (parentBody->parentBody != NULL) {
            newStateP = parentBody->parentBody->getStateAtTime(propagationData[i].JDTDB);
            newState[0] += newStateP[0];
        }
        lineData.push_back(propagationData[i].position[0] - newState[0][0]);
        lineData.push_back(propagationData[i].position[1] - newState[0][1]);
        lineData.push_back(propagationData[i].position[2] - newState[0][2]);
        lineData.push_back(0.8);
        lineData.push_back(0.8);
        lineData.push_back(0.4);
    }

    lineDataSize = (int)lineData.size() / 6;

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, lineDataSize * 6 * sizeof(double), lineData.data(), GL_STATIC_DRAW);
    glVertexAttribLPointer(0, 3, GL_DOUBLE, sizeof(double) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribLPointer(1, 3, GL_DOUBLE, sizeof(double) * 6, (void*)(3 * sizeof(double)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void Spacecraft::updateOrbitLines(PlanetaryBody* parent){
    parentBody = parent;
    lineMat->matShader->use();

    std::vector<double> lineData;
    statevec newState;
    statevec newStateP;
    for (int i = 0; i < propagationData.size(); ++i) {
        newState = parentBody->getStateAtTime(propagationData[i].JDTDB);
        if (parentBody->parentBody != NULL) {
            newStateP = parentBody->parentBody->getStateAtTime(propagationData[i].JDTDB);
            newState[0] += newStateP[0];
        }
        lineData.push_back(propagationData[i].position[0] - newState[0][0]);
        lineData.push_back(propagationData[i].position[1] - newState[0][1]);
        lineData.push_back(propagationData[i].position[2] - newState[0][2]);
        lineData.push_back(0.8);
        lineData.push_back(0.8);
        lineData.push_back(0.4);
    }

    lineDataSize = (int)lineData.size() / 6;

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, lineDataSize * 6 * sizeof(double), lineData.data(), GL_STATIC_DRAW);
    glVertexAttribLPointer(0, 3, GL_DOUBLE, sizeof(double) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribLPointer(1, 3, GL_DOUBLE, sizeof(double) * 6, (void*)(3 * sizeof(double)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void Spacecraft::interpToTime(double julian) {
    spacecraftState newState = interpEphIndex(julian);
    this->setPosition(newState.position);
    this->setVelocity(newState.velocity);
    this->setAttitude(newState.attitude);

}

spacecraftState Spacecraft::interpEphIndex(double julian) {
    unsigned long min, max, mid;
    min = 0;
    max = propagationData.size()-1;
    
    // a quick and dirty binary searching algorithm to find
    // the appropriate states to interpolate for the given time
    spacecraftState retState;
    while( max-min > 1){
        mid = (max+min)/2;

        if(propagationData[mid].JDTDB > julian){
            max = mid;
        }
        else{
            min = mid;
        }
    }

    return interpScState(propagationData[min], propagationData[max], julian);
}

void Spacecraft::draw() {
    if(bodyMat != NULL){
        bodyMat->draw();
    }
    if (lineMat != NULL) {
        glEnable(GL_DEPTH_TEST);
        lineMat->matShader->use();
        mat4 modelMat;
        modelMat = (parentBody!=NULL) ? TransMat(parentBody->position) : eye4();
        lineMat->matShader->setUniform4dv("model", &modelMat[0][0]);
        glBindVertexArray(lineVAO);
        glDrawArrays(GL_LINE_STRIP, 0, lineDataSize);

        glPointSize(2.0);
        glDrawArrays(GL_POINTS, 0, lineDataSize);
        glBindVertexArray(0);
    }
}

void Spacecraft::setUniform4dv(const char * uniformName, double * uniformPtr) {
    if (bodyMat != NULL)
        bodyMat->matShader->setUniform4dv(uniformName, uniformPtr);
    if (lineMat != NULL)
        lineMat->matShader->setUniform4dv(uniformName, uniformPtr);
}

void Spacecraft::setUniformV3(const char * uniformName, float * vec) {
    if (bodyMat != NULL)
        bodyMat->matShader->setUniformV3(uniformName, vec);
    if (lineMat != NULL)
        lineMat->matShader->setUniformV3(uniformName, vec);
}

void Spacecraft::setUniformV3(const char * uniformName, double * vec) {
    if (bodyMat != NULL)
        bodyMat->matShader->setUniformV3(uniformName, vec);
    if (lineMat != NULL)
        lineMat->matShader->setUniformV3(uniformName, vec);
}

void Spacecraft::setUniformV3(const char * uniformName, vec3 vec) {
    if (bodyMat != NULL)
        bodyMat->matShader->setUniformV3(uniformName, vec);
    if (lineMat != NULL)
        lineMat->matShader->setUniformV3(uniformName, vec);
}

void Spacecraft::setUniformV3(const char * uniformName, vec3f vec) {
    if (bodyMat != NULL)
        bodyMat->matShader->setUniformV3(uniformName, vec);
    if (lineMat != NULL)
        lineMat->matShader->setUniformV3(uniformName, vec);
}
