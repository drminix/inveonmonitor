#include "gateutility.h"
#include<cstdlib>
#include<cstdio>
#include<cstring>
#include<QFile>
#include<QTextStream>
#include<QProcess>
GateUtility::GateUtility()
{
}



//run simulation
bool GateUtility::runSimulation(JSOutput* jsoutput,QString cluster,int startmachine,int requiredmachine) {
   /* //let's create a hrun file
    QString hruntemplate="#!/bin/bash\n"
"# hrun $1 $2\n"
"# $1 = sciprt name\n"
"# $2 = destination\n"
"MACHINE=%1\n"
"SCRIPT=bunch\n"
"FOLDER=$(pwd)\n"
"##change permissions\n"
"cd $FOLDER\n"
"chmod +x *.sh\n"
"##running simulations\n"
"echo \"hrun: running simulations..\"\n"
"for MNUMBER in {%2..%3..1}\n"
"do\n"
"echo \"$MACHINE$MNUMBER\"\n"
"#change the script number to 3 digit\n"
"SNUMBER=$((MNUMBER-%2))\n";
    QString bottomhalf=
"SCRIPTNUMBER=$(printf \"%03d\" $SNUMBER)\n"
"ssh $MACHINE$MNUMBER ${FOLDER}/${SCRIPT}${SCRIPTNUMBER}.sh &\n"
"done\n"
;

   QFile file(QString("%1/hrun").arg(jsoutput->PWD));
   file.open(QFile::WriteOnly|QFile::Truncate);

   QTextStream stream(&file);
   stream<<hruntemplate.arg(cluster,QString::number(startmachine,10),QString::number(requiredmachine+startmachine-1,10));
   stream<<bottomhalf;
   file.close();
   file.setPermissions((QFile::Permission)0x7000);
   QProcess* qprocess = new QProcess();
   qprocess->start(file.fileName());
   qprocess->waitForFinished();

*/
    //temporary fix for UTMC machine
    QFile file(QString("%1/bunch000.sh").arg(jsoutput->PWD));
    QProcess* qprocess = new QProcess();
    qprocess->start(file.fileName());
    qprocess->waitForFinished();
}


//validate and insecpt the script
TimeInfo GateUtility::inspect(QString input) {
    TimeInfo info;

    QFile inputFile(input);
    inputFile.open(QFile::ReadOnly); //read only

    QTextStream stream;
    stream.setDevice(&inputFile);
    while(stream.atEnd()!=true) {
        QString qtline = stream.readLine();
        const char* line = qtline.toStdString().c_str();
        /*
        /gate/application/setTimeSlice      45  s # # 45 seconds * 60 projections
        /gate/application/setTimeStart      0    s #
        /gate/application/setTimeStop      2700 s # 45 minutes
         */
          printf("read: %s",line);
        if(strncmp(line,"/gate/application/setTimeSlice",30)==0) {
          //write the next three lines.
          sscanf(line+30,"%f",&info.perslice);
        }else if(strncmp(line,"/gate/application/setTimeStop",29)==0) {
          sscanf(line+29,"%f",&info.totalTime);
      }
    }

    if(info.perslice!=0&&info.totalTime!=0) {
        info.nop = info.totalTime / info.perslice;
        info.valid = true;
    }

    return info;

}

JSOutput* GateUtility::js(JSInput* input) {

#define ARRAYZ 255
#define POSTFIX_LENGTH 10
#define SCRIPT_LENGTH 200
    JSOutput* jsoutput = new JSOutput();

     int i;
     FILE* in;
     char line[ARRAYZ];
     FILE** out;
     FILE** outscriptfs;
     double totalTime;
     double projectionTime;
     int nop;
     char **outputfile;
     char **outputscript;
     int outputlen;
     char line2[ARRAYZ];
     char line3[ARRAYZ];
     int ncore=1;
     int ncore_num;
     FILE** ncoreout;
     char** ncorefile;
     int j;

     //setup data
     totalTime = input->totalTime;
     projectionTime = input->perProjectionSlice;
     nop = (float)input->numberOfProjections;
     ncore = input->numberOfCore;
     ncore_num = nop/ncore;

     if(nop%ncore!=0) ncore_num++;

     printf("Total time: %lf seconds\n",totalTime);
     printf("Per projection: %lf seconds\n",projectionTime);
     printf("number of projections: %d\n",nop);
     printf("number of cores: %d\n",ncore);
     printf("nop/ncore: %d\n",ncore_num);
     outputlen = input->outputPrefix.length();

     //create output file name
     outputfile = (char**)malloc(sizeof(char*) * nop);
     outputscript = (char**)malloc(sizeof(char*) * nop);
     for(i=0;i<nop;i++) {
       outputfile[i] = (char*)malloc(sizeof(char)*(outputlen+POSTFIX_LENGTH));
       outputscript[i] = (char*)malloc(sizeof(char)*(SCRIPT_LENGTH));
       sprintf(outputfile[i],"%s_%03d.mac\0",input->outputPrefix.toStdString().c_str(),i);
       sprintf(outputscript[i],"%s/script_%03d.sh\0",input->PWD.toStdString().c_str(),i);
       jsoutput->clusterFiles.append(QString(outputfile[i]));
       jsoutput->scriptFiles.append(QString(outputscript[i]));
     }


     ncorefile = (char**)malloc(sizeof(char*)*ncore_num);
     ncoreout = (FILE**)malloc(sizeof(FILE*)*ncore_num);
     for(i=0;i<ncore_num;i++) {

        ncorefile[i] = (char*)malloc(sizeof(char*)*(SCRIPT_LENGTH));
        sprintf(ncorefile[i],"%s/bunch%03d.sh",input->PWD.toStdString().c_str(),i);
        jsoutput->batchFiles.append(QString(ncorefile[i]));
        ncoreout[i] = (FILE*)malloc(sizeof(FILE*));
        ncoreout[i] = fopen(ncorefile[i],"w+");
        if(ncoreout[i]==NULL) {
            printf("ERROR: while creating %s\n",ncorefile[i]);
         exit(-1);
       }
        QFile file(QString(ncorefile[i]));
        file.setPermissions((QFile::Permission)0x7000);

     }

     //(1) open input and output files
     in = fopen(input->inputFile.toStdString().c_str(),"r");
     if(in==NULL) {
       printf("ERROR: %s does not exist!\n",input->inputFile.toStdString().c_str());
       exit(-1);
     }
     out = (FILE**)malloc(sizeof(FILE*)*nop);
     outscriptfs = (FILE**)malloc(sizeof(FILE*)*nop);
     for(i=0;i<nop;i++) {
       out[i] = (FILE*)malloc(sizeof(FILE*));
       out[i] = fopen(outputfile[i],"w+");
       outscriptfs[i] = (FILE*)malloc(sizeof(FILE*));
       outscriptfs[i] = fopen(outputscript[i],"w+");
       if(out[i]==NULL||outscriptfs[i]==NULL) {
         printf("ERROR: while creating %s or scirpt\n",outputfile[i]);
         exit(-1);
       }
     }

     //(2) process the file
     while(fgets(line,ARRAYZ,in)!=NULL) {

     //time information
     /*
     /gate/application/setTimeSlice      45  s # # 45 seconds * 60 projections
     /gate/application/setTimeStart      0    s #
     /gate/application/setTimeStop      2700 s # 45 minutes
      */
       printf("read: %s",line);
     if(strncmp(line,"/gate/application/setTimeSlice",30)==0) {
       //write the next three lines.
       sprintf(line,"/gate/application/setTimeSlice %lf s",projectionTime);
       for(i=0;i<nop;i++) {
         fprintf(out[i],"%s\n",line);
       }
     }else if(strncmp(line,"/gate/application/setTimeStart",30)==0) {
       //time start is different
       for(i=0;i<nop;i++) {
         sprintf(line,"/gate/application/setTimeStart %lf s",projectionTime*i);
         fprintf(out[i],"%s\n",line);
       }
     }else if(strncmp(line,"/gate/application/setTimeStop",29)==0) {
       //time stop is also different
       for(i=0;i<nop;i++) {
         sprintf(line,"/gate/application/setTimeStop %lf s",projectionTime*(i+1));
         fprintf(out[i],"%s\n",line);
       }
   //gate/output/root/setFileName /output2/slee91/cobalt57_1MGP10_25
     }else if(strncmp(line,"/gate/output/root/setFileName",29)==0) {
       sscanf(line,"%s %s",line2,line3);
       for(i=0;i<nop;i++) {
         sprintf(line,"%s %s_%03d" ,line2,line3,i);
         fprintf(out[i],"%s\n",line);
       }
       ///gate/output/projection/setFileName /output2/slee91/cobalt57_1MGP10_25_interfile
     }else if(strncmp(line,"/gate/output/projection/setFileName",35)==0) {
       sscanf(line,"%s %s",line2,line3);
       for(i=0;i<nop;i++) {
         sprintf(line,"%s %s_%03d", line2,line3,i);
         fprintf(out[i],"%s\n",line);
       }
   //
     }else if(strncmp(line,"/gate/output/ascii/setFileName",30)==0) {
         sscanf(line,"%s %s",line2,line3);
       for(i=0;i<nop;i++) {
         sprintf(line,"%s %s_%03d", line2,line3,i);
         fprintf(out[i],"%s\n",line);
       }
     }else {
       //just copy the data
       //printf("just copy it\n");
       for(i=0;i<nop;i++) {
         fprintf(out[i],"%s",line);
       }
     }


     }

     //write script.sh
     for(i=0;i<nop;i++) {
       fprintf(outscriptfs[i],"cd %s\n",input->PWD.toStdString().c_str());
       fprintf(outscriptfs[i],"taskset -c %d Gate %s_%03d.mac &\n",i, input->outputPrefix.toStdString().c_str(),i);
          fprintf(outscriptfs[i],"disown -h\n");
     }

     //take care of the special case when the number of projection is a multiple of number of cores.
     if((nop%ncore)==0) ncore_num++;

     for(i=0;i<ncore_num-1;i++) {
       for(j=0;j<ncore;j++) {
         fprintf(ncoreout[i],"%s/script_%03d.sh &\n",input->PWD.toStdString().c_str(),i*ncore+j);
       }
       fclose(ncoreout[i]);
     }

     //write the rest
     bool more=false;
     for(j=0;j<(nop%ncore);j++) {
       fprintf(ncoreout[i],"%s/script_%03d.sh &\n",input->PWD.toStdString().c_str(),i*ncore+j);
       more=true;

     }
     //fprintf(ncoreout[i],"disown -h");
    // fprintf(ncoreout[i],"ps aux | grep Gate");
     if(more==true) fclose(ncoreout[i]);

     //close files
     fclose(in);
     for(i=0;i<nop;i++) {
       fclose(out[i]);
       fclose(outscriptfs[i]);
     }

     //make it executable
     for(i=0;i<nop;i++) {
         QFile file(QString(outputscript[i]));
         file.setPermissions((QFile::Permission)0x7000);
     }
     jsoutput->PWD = input->PWD;
     jsoutput->valid = true;
     return jsoutput;

}

JSOutput* GateUtility::jsForCluster(JSInput* input) {

#define ARRAYZ 255
#define POSTFIX_LENGTH 10
#define SCRIPT_LENGTH 200
    JSOutput* jsoutput = new JSOutput();

     int i;
     FILE* in;
     char line[ARRAYZ];
     FILE** out;
     FILE** outscriptfs;
     double totalTime;
     double projectionTime;
     int nop;
     char **outputfile;
     char **outputscript;
     int outputlen;
     char line2[ARRAYZ];
     char line3[ARRAYZ];
     int ncore;
     int ncore_num;
     FILE** ncoreout;
     char** ncorefile;
     int j;

     //setup data
     totalTime = input->totalTime;
     projectionTime = input->perProjectionSlice;
     nop = (float)input->numberOfProjections;
     ncore = input->numberOfCore;
     ncore_num = nop/ncore;

     if(nop%ncore!=0) ncore_num++;

     printf("Total time: %lf seconds\n",totalTime);
     printf("Per projection: %lf seconds\n",projectionTime);
     printf("number of projections: %d\n",nop);
     printf("number of cores: %d\n",ncore);
     printf("nop/ncore: %d\n",ncore_num);
     outputlen = input->outputPrefix.length();

     //create output file name
     outputfile = (char**)malloc(sizeof(char*) * nop);
     outputscript = (char**)malloc(sizeof(char*) * nop);
     for(i=0;i<nop;i++) {
       outputfile[i] = (char*)malloc(sizeof(char)*(outputlen+POSTFIX_LENGTH));
       outputscript[i] = (char*)malloc(sizeof(char)*(SCRIPT_LENGTH));
       sprintf(outputfile[i],"%s_%03d.mac\0",input->outputPrefix.toStdString().c_str(),i);
       sprintf(outputscript[i],"%s/script_%03d.sh\0",input->PWD.toStdString().c_str(),i);
       jsoutput->clusterFiles.append(QString(outputfile[i]));
       jsoutput->scriptFiles.append(QString(outputscript[i]));
     }


     ncorefile = (char**)malloc(sizeof(char*)*ncore_num);
     ncoreout = (FILE**)malloc(sizeof(FILE*)*ncore_num);
     for(i=0;i<ncore_num;i++) {
        ncorefile[i] = (char*)malloc(sizeof(char*)*(SCRIPT_LENGTH));
        sprintf(ncorefile[i],"%s/bunch%03d.sh",input->PWD.toStdString().c_str(),i);
        jsoutput->batchFiles.append(QString(ncorefile[i]));
        ncoreout[i] = (FILE*)malloc(sizeof(FILE*));
        ncoreout[i] = fopen(ncorefile[i],"w+");
        if(ncoreout[i]==NULL) {
            printf("ERROR: while creating %s\n",ncorefile[i]);
         exit(-1);
       }
        QFile file(QString(ncorefile[i]));
        file.setPermissions((QFile::Permission)0x7000);

     }

     //(1) open input and output files
     in = fopen(input->inputFile.toStdString().c_str(),"r");
     if(in==NULL) {
       printf("ERROR: %s does not exist!\n",input->inputFile.toStdString().c_str());
       exit(-1);
     }
     out = (FILE**)malloc(sizeof(FILE*)*nop);
     outscriptfs = (FILE**)malloc(sizeof(FILE*)*nop);
     for(i=0;i<nop;i++) {
       out[i] = (FILE*)malloc(sizeof(FILE*));
       out[i] = fopen(outputfile[i],"w+");
       outscriptfs[i] = (FILE*)malloc(sizeof(FILE*));
       outscriptfs[i] = fopen(outputscript[i],"w+");
       if(out[i]==NULL||outscriptfs[i]==NULL) {
         printf("ERROR: while creating %s or scirpt\n",outputfile[i]);
         exit(-1);
       }
     }

     //(2) process the file
     while(fgets(line,ARRAYZ,in)!=NULL) {

     //time information
     /*
        [usage] /gate/application/startDAQCluster virtualStart virtualStop dummy unit

     */
       printf("read: %s",line);
     if(strncmp(line,"/gate/application/startDAQ",26)==0) {
        for(i=0;i<nop;i++) {
        sprintf(line,"/gate/application/startDAQCluster %lf %lf 0 s",projectionTime*i,projectionTime*(i+1));
        fprintf(out[i],"%s\n",line);
         }

   //gate/output/root/setFileName /output2/slee91/cobalt57_1MGP10_25
     }else if(strncmp(line,"/gate/output/root/setFileName",29)==0) {
       sscanf(line,"%s %s",line2,line3);
       for(i=0;i<nop;i++) {
         sprintf(line,"%s %s_%03d" ,line2,line3,i);
         fprintf(out[i],"%s\n",line);
       }
       ///gate/output/projection/setFileName /output2/slee91/cobalt57_1MGP10_25_interfile
     }else if(strncmp(line,"/gate/output/projection/setFileName",35)==0) {
       sscanf(line,"%s %s",line2,line3);
       for(i=0;i<nop;i++) {
         sprintf(line,"%s %s_%03d", line2,line3,i);
         fprintf(out[i],"%s\n",line);
       }
   //
     }else if(strncmp(line,"/gate/output/ascii/setFileName",30)==0) {
         sscanf(line,"%s %s",line2,line3);
       for(i=0;i<nop;i++) {
         sprintf(line,"%s %s_%03d", line2,line3,i);
         fprintf(out[i],"%s\n",line);
       }
     }else {
       //just copy the data
       //printf("just copy it\n");
       for(i=0;i<nop;i++) {
         fprintf(out[i],"%s",line);
       }
     }

     }

     //write script.sh
     for(i=0;i<nop;i++) {
       fprintf(outscriptfs[i],"cd %s\n",input->PWD.toStdString().c_str());
       fprintf(outscriptfs[i],"Gate %s_%03d.mac &\n",input->outputPrefix.toStdString().c_str(),i);
           fprintf(outscriptfs[i],"disown -h\n");
     }

     //take care of the special case when the number of projection is a multiple of number of cores.
     if((nop%ncore)==0) ncore_num++;

     for(i=0;i<ncore_num-1;i++) {
       for(j=0;j<ncore;j++) {
         fprintf(ncoreout[i],"%s/script_%03d.sh &\n",input->PWD.toStdString().c_str(),i*ncore+j);
       }
       fclose(ncoreout[i]);
     }

     //make it executable
     for(i=0;i<ncore_num-1;i++) {
        QFile file(QString(ncorefile[i]));
        file.setPermissions((QFile::Permission)0x7000);

     }

     //write the rest
     bool more=false;
     for(j=0;j<(nop%ncore);j++) {
       fprintf(ncoreout[i],"%s/script_%03d.sh &\n",input->PWD.toStdString().c_str(),i*ncore+j);
       more=true;

     }
     //fprintf(ncoreout[i],"disown -h");
    // fprintf(ncoreout[i],"ps aux | grep Gate");
     if(more==true) fclose(ncoreout[i]);

     //close files
     fclose(in);
     for(i=0;i<nop;i++) {
       fclose(out[i]);
       fclose(outscriptfs[i]);
     }

     //make it executable
     for(i=0;i<nop;i++) {
         QFile file(QString(outputscript[i]));
         file.setPermissions((QFile::Permission)0x7000);
     }

     jsoutput->PWD = input->PWD;
     jsoutput->valid = true;
     return jsoutput;

}


void GateUtility::killIt(QString where, QString pid) {
    QString command("ssh %1 kill %2");

    QProcess *process = new QProcess();
    process->start(command.arg(where,pid));
    process->waitForFinished();

}
