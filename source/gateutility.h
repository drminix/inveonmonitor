#ifndef GATEUTILITY_H
#define GATEUTILITY_H

#include<QStringList>

typedef struct JSOutput {
    QStringList clusterFiles;
    QStringList scriptFiles;
    QStringList batchFiles;
    QString PWD;
    bool valid;
} JSOutput;

typedef struct JSInput {
    QString inputFile;
    QString outputPrefix;
    float totalTime;
    float perProjectionSlice;
    int numberOfCore;
    int numberOfProjections;
    QString PWD;
} JSInput;

typedef struct TimeInfo {
    float totalTime;
    int nop;
    float perslice;
    bool valid;

    TimeInfo() :totalTime(0), nop(0), perslice(0), valid(false) {

    }
} TimeInfo;

class GateUtility
{
public:
    GateUtility();

    //split simulations
    JSOutput* js(JSInput*);                   //js
    JSOutput* jsForCluster(JSInput*);         //js for cluster

    TimeInfo inspect(QString );
    bool runSimulation(JSOutput* jsoutput,QString cluster,int startmachine,int requiredmachine);
    void killIt(QString where,QString pid);

};

#endif // GATEUTILITY_H
