#include "Mission.h"

// Propagation functions


vec3 motionEqnBarycentric(double time, vec3 position) {
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

spacecraftState Stage::dynamicsEquation(double time, spacecraftState cs, int stepFlag, bool& isFiring){
    spacecraftState ds;
    ds.JDTDB = 1; // dt/dt
    vec3 aocsTorque, aocsThrust, omegaB, acceleration;
    AOCSSoftware(this, cs, time, aocsTorque, aocsThrust, isFiring);


    switch(stepFlag){
    case STEP_ATT:
        ds.angularVel = inertialInverse(cs.inertialMat)*(aocsTorque - Cross(cs.angularVel, cs.inertialMat*cs.angularVel));
        ds.attitude =  (crossMat(cs.angularVel)*-1.0) * cs.attitude;
        break;

    case STEP_POS:
        ds.position = cs.velocity; // dx/dt
        acceleration = motionEqnBarycentric(cs.JDTDB, cs.position);
        ds.velocity = acceleration;// dv/dt
        break;

    case STEP_FULL:
        double unitConv = 86400.0*86400.0 / (1000.0);
        ds.mass = -( this->maxMdot * len(aocsThrust) / this->maxThrust)*86400.0;// dm/dt
        ds.inertialMat = zeros3();// dJ/dt
        ds.position = cs.velocity; // dx/dt
        acceleration = motionEqnBarycentric(cs.JDTDB, cs.position);
        acceleration += ((transpose(cs.attitude) * aocsThrust)/cs.mass)*unitConv; 
        ds.velocity = acceleration;// dv/dt
        ds.angularVel = inertialInverse(cs.inertialMat)*(aocsTorque - Cross(cs.angularVel, cs.inertialMat*cs.angularVel));
        ds.attitude =  (crossMat(cs.angularVel)*-1.0) * cs.attitude;
        ds.thrust = len(aocsThrust);
        break;
    }
    
    return ds;
}


// ---------------------------MISSION--------------------------------

Mission::Mission(string name, spacecraftState initState,vector<aocsFun> Softwares, vector<vector<maneuver>> maneuvers){
    missionName = name;
    initialConditions = initState;
    this->Softwares = Softwares;
    this->maneuvers = maneuvers;
    texHandler->loadTexture("Fire",true,false);
}

void Mission::addSubstage( string texMod, double wetMass, double dryMass, double negZ, double posZ, double ISP, double maxThrust, double maxMdot, mat3 inertialMat, stageEvent stgevnt){
    substage stg;
    texHandler->loadTexture(texMod, true, false);
    stg.name = texMod;
    stg.obj = new SceneObject(texMod);
    stg.obj->setScale(0.001);
    stg.mat = new Material("DS_1", texMod);
    stg.mat->addObj(stg.obj);

    stg.thrustObj = new SceneObject(texMod+"Thrust");
    stg.thrustObj->setScale(0.001);
    stg.thrustMat = new Material("Fire", "Fire");
    stg.thrustMat->addObj(stg.thrustObj);

    stg.dryMass = dryMass;
    stg.wetMass = wetMass;
    stg.specificImpulse = ISP;
    stg.maxThrust = maxThrust;
    stg.maxMdot = maxMdot;
    stg.negz = negZ;
    stg.posz = posZ;
    stg.inertialMat = inertialMat;

    stg.mat->matShader->setUniformV3("sun.color", vec3f{ 1.0f,1.0f,0.95f });
    stg.mat->matShader->setUniformV3("sun.position", vec3{ 0.0,0.0,0.0 });
    stg.mat->matShader->setUniform1f("sun.attn_str", 5.0);
    stg.mat->matShader->setUniform1f("sun.attn_lin", 5e-9);
    stg.mat->matShader->setUniform1f("sun.attn_quad", 0);

    stg.terminate = stgevnt;

    this->substages.push_back(stg);
}

void Mission::propagate(double MET){
    spacecraftState stgInput = initialConditions; 
    double T = stgInput.JDTDB;
    
    auto tempSubStages = this->substages;
    uint nrSubStages = this->substages.size();

    for(uint i = 0; i<nrSubStages; ++i){
        //Create a new stage from the full list of stages left in tempSubStages
        Stage* groupedStage = new Stage(tempSubStages, Softwares[Softwares.size()-1], maneuvers[maneuvers.size()-1]);
        Softwares.pop_back();
        maneuvers.pop_back();
        //propagate it using the data in stgInput (starts as initial conditions, but gets set to the end of each previous stage)
        stgInput.mass = groupedStage->getWetMass();
        cout << stgInput.mass << endl;
        groupedStage->propagate(stgInput);
        this->stages.push_back(groupedStage);
        
        // Update the initial conditions for the new grouped stage
        stgInput = groupedStage->getEndState();
        // the new grouped stage mass should be the 

        //Create a stage from the last element in the substage vector.
        //make sure it is not duplicating the last substage
        if(i != nrSubStages-1){
            //create a vector to initiate the Stage with
            vector<substage> spentStageVec;
            //add the last substage (the spent one)
            spentStageVec.push_back(tempSubStages[tempSubStages.size()-1]);
            //create the Stave with the single substage
            Stage* spentStage = new Stage(spentStageVec, Softwares[Softwares.size()-1], maneuvers[maneuvers.size()-1]);
            Softwares.pop_back();
            maneuvers.pop_back();
            //get the end state of the full grouped stage (which will be appx the starting state for the spent stage)
            spacecraftState spentStageInitState = groupedStage->getEndState();
            //Adjust the initialization state to be the new condition.
            spentStageInitState.velocity = spentStageInitState.velocity + 1.0e1 * (spentStageInitState.attitude * vec3{-1.0,0.0,0.0} );
            spentStageInitState.mass = tempSubStages[tempSubStages.size()-1].wetMass - (groupedStage->getStartState().mass - groupedStage->getEndState().mass);
            uint nrSubStages = tempSubStages.size();
            double heightOffset = 0.0;
            for(int i = 0; i<nrSubStages; ++i){
                if(i!=nrSubStages-1){
                    heightOffset += (this->substages[i+1].posz - this->substages[i].negz);
                }
            }
            vec3 offsetPos = spentStageInitState.attitude * (heightOffset*vec3{-1.0,0.0,0.0});
            spentStageInitState.position += offsetPos;
            spentStage->whenToStage.value = MET - (spentStageInitState.JDTDB - T);
            //Propagate the spent stage until the end of the mission duration
            cout << spentStageInitState.mass << endl;
            spentStage->propagate(spentStageInitState);
            this->stages.push_back(spentStage);

            // subtract the mass of the spent stage from the initial conditions of the next grouped stage
            stgInput.mass -= spentStageInitState.mass;
        }
        // remove the spent stage from the tempSubStages vector. The grouped stage for the next
        // step should now be everything minus the spent stage
        tempSubStages.pop_back();
        
    }
    solarSystem->missions[this->missionName] = this;
    
}
void Mission::drawCraft(){
    for (auto const& stg : this->stages){
        stg->drawCraft();
    }
}
void Mission::drawLines(){
    for (auto const& stg : this->stages){
        stg->drawLines();
    }
}

void Mission::setUniform4dv(const char* uniformName, double* uniformPtr){
    for(auto const& stg : this->stages){
        if(stg->lineMat!=NULL){
            stg->lineMat->matShader->setUniform4dv(uniformName, uniformPtr);
        }
    }
}
void Mission::setUniform4dvCraft(const char* uniformName, double* uniformPtr){
    for(auto const& substg : this->substages){
        substg.mat->matShader->setUniform4dv(uniformName, uniformPtr);
        substg.thrustMat->matShader->setUniform4dv(uniformName, uniformPtr);
    }
}
void Mission::setUniformV3(const char* uniformName, float* uniformPtr){
    for(auto const& substg : this->substages){
        substg.mat->matShader->setUniformV3(uniformName, uniformPtr);
        substg.thrustMat->matShader->setUniformV3(uniformName, uniformPtr);
    }
    for(auto const& stg : this->stages){
        if(stg->lineMat!=NULL){
            stg->lineMat->matShader->setUniformV3(uniformName, uniformPtr);
        }
    }
}
void Mission::setUniformV3(const char* uniformName, double* uniformPtr){
    for(auto const& substg : this->substages){
        substg.mat->matShader->setUniformV3(uniformName, uniformPtr);
        substg.thrustMat->matShader->setUniformV3(uniformName, uniformPtr);
    }
    for(auto const& stg : this->stages){
        if(stg->lineMat!=NULL){
            stg->lineMat->matShader->setUniformV3(uniformName, uniformPtr);
        }
    }
}
void Mission::setUniformV3(const char* uniformName, vec3& uniformPtr){
    for(auto const& substg : this->substages){
        substg.mat->matShader->setUniformV3(uniformName, uniformPtr);
        substg.thrustMat->matShader->setUniformV3(uniformName, uniformPtr);
    }
    for(auto const& stg : this->stages){
        if(stg->lineMat!=NULL){
            stg->lineMat->matShader->setUniformV3(uniformName, uniformPtr);
        }
    }
}
void Mission::setUniformV3(const char* uniformName, vec3f& uniformPtr){
    for(auto const& substg : this->substages){
        substg.mat->matShader->setUniformV3(uniformName, uniformPtr);
        substg.thrustMat->matShader->setUniformV3(uniformName, uniformPtr);
    }
    for(auto const& stg : this->stages){
        if(stg->lineMat!=NULL){
            stg->lineMat->matShader->setUniformV3(uniformName, uniformPtr);
        }
    }
}
//#################   STAGE    ##################################

Stage::Stage(vector<substage> stgs, aocsFun Software, vector<maneuver> maneuvers){

    this->AOCSSoftware = Software;
    this->maneuvers = maneuvers;

    // Determine wet/dry/fuel mass of the stage
    this->substages = stgs;
    int endex = stgs.size()-1;
    
    for(auto const stg : stgs){
        this->wetMass += stg.wetMass;
    }
    this->dryMass = this->wetMass - (stgs[endex].wetMass - stgs[endex].dryMass);
    this->fuelMass = this->wetMass - this->dryMass;

    // Specific impulse
    this->specificImpulse = stgs[endex].specificImpulse;
    this->maxMdot = stgs[endex].maxMdot;
    this->maxThrust = stgs[endex].maxThrust;
    this->whenToStage = stgs[endex].terminate;

    // add: calculate inertial matrix using data from substages

    

}
double Stage::getYeet(){
    return this->timeOfYeet;
}
double Stage::getYote(){
    return this->timeOfYote;
}
void Stage::propagate(spacecraftState initstate){
    //initstate.mass = this->wetMass;
    //initstate.inertialMat = this->inertialMat;
    this->timeOfYeet = initstate.JDTDB;
    printf("Propagating stage...");
    missionStateFunc propFun = [this](double time, spacecraftState cs, int stpType, bool& firing){return this->dynamicsEquation(time, cs, stpType, firing);};
    vec3_rkf45_stg(initstate, whenToStage, 1.0e-5, propFun, &propagationData);
    printf("done!\n");
    this->timeOfYote = propagationData[propagationData.size()-1].JDTDB;
    
    
}

void Stage::genOrbitLines(PlanetaryBody* parent) {
    parentBody = parent;
    lineMat = new Material("line", "NULL");
    lineMat->matShader->use();
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);


    std::vector<double> lineData;
    statevec newState;
    statevec newStateP;
    for (int i = 0; i < propagationData.size(); i+=60) {
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


void Stage::drawLines(){
    if(lineMat != NULL){
        glEnable(GL_DEPTH_TEST);
        lineMat->matShader->use();
        mat4 modelMat = (parentBody!=NULL) ? TransMat(parentBody->position) : eye4();
        lineMat->matShader->setUniform4dv("model", &modelMat[0][0]);
        glBindVertexArray(lineVAO);
        glDrawArrays(GL_LINE_STRIP, 0, lineDataSize);

        //glDisable(GL_DEPTH_TEST);
        //glPointSize(5.0);
        //glDrawArrays(GL_POINTS, 0, lineDataSize);

        glBindVertexArray(0);
    }
}
void Stage::drawCraft(){
    if(shouldDraw){
        uint nrSubStages = this->substages.size();
        double heightOffset = 0.0;
        for(int i = 0; i<nrSubStages; ++i){
            vec3 rootPos = this->substages[0].obj->getPos(); //orbiter position
            vec3 offsetPos = this->attitude * (heightOffset*vec3{-1.0,0.0,0.0});
            this->substages[i].obj->setPos(rootPos + offsetPos);
            this->substages[i].mat->draw();
            
            if(this->firing && i == nrSubStages-1){
                this->substages[i].thrustObj->setPos(rootPos + offsetPos);
                this->substages[i].thrustMat->draw();
            }
            if(i!=nrSubStages-1){
                heightOffset += (this->substages[i+1].posz - this->substages[i].negz);
            }
        }
    }
}


void Stage::interpToTime(double julian) {
    spacecraftState newState = interpEphIndex(julian);
    if(newState.thrust > 0.0)
        this->firing = true;
    else
        this->firing = false;


    this->setPosition(newState.position);
    this->setVelocity(newState.velocity);
    this->setAttitude(newState.attitude);
    this->curMass = newState.mass;
    this->curVel = (newState.velocity)/86400.0;
    this->curOmega = len(newState.angularVel)/86400.0;
    this->curThrust = newState.thrust;
}

void Stage::setPosition(vec3 pos) {
    this->position = pos;
    for(auto const& stg : substages){
        stg.obj->setPos(pos);
        stg.thrustObj->setPos(pos);
    }
}

void Stage::setVelocity(vec3 vel) {
    this->velocity = vel;
}
void Stage::setAttitude(mat3 att){
    this->attitude = att;
    for(auto const& stg : substages){
        stg.obj->setAttitude(mat3expand(att));
        stg.thrustObj->setAttitude(mat3expand(att));
    }
}

spacecraftState Stage::interpEphIndex(double julian) {
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

spacecraftState Stage::getEndState(){
    return propagationData[propagationData.size()-1];
}

spacecraftState Stage::getStartState(){
    return propagationData[0];
}

