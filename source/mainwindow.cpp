#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDataStream>
#include "Configuration.h"
#include <cmath>
#include <vector>

using std::vector;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initialize();
}

void MainWindow::initialize() {
    //Run Simulation TAB
    //initialize Taget Cluster Combo Box
    ui->comboCluster->insertItem(0,QString("UTMC"));
    ui->comboCluster->insertItem(0,QString("hydra"));
    ui->comboCluster->insertItem(1,QString("arc"));

    //PER NODE TAB
    //initialize combo Box;
    QString base("UTMC");

    ui->nodeCombo->insertItem(0,base);

    //initialize utility
    this->utility = new GateUtility();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_nodeCombo_currentIndexChanged(QString host)
{

   updateView(host);

}

void MainWindow::updateView(QString host) {


    QString pscommand= "p";

    //runCommand & get output
    QProcess *myProcess = new QProcess(this);
    myProcess->start(pscommand); //start the process

    myProcess->waitForFinished();

    QString result(myProcess->readAll());
   
    QStringList processed;
    processed = result.split("\n");

    //clear the listView
    ui->nodeList->clear();

    //add listview
    QStringList::const_iterator iter;
    QString output;
    QString line;

    for(iter = processed.constBegin(); iter != processed.constEnd(); iter++) {

        line=(*iter).toLocal8Bit().constData();
        if(line.contains("Gate")) {
            //skip "grep Gate"
            if(line.contains("grep Gate")==true) {
                continue;
            }else if(line.contains("==>")==true) {
                continue;
            }
          QListWidgetItem *newItem = new QListWidgetItem(line);
          ui->nodeList->insertItem(0,newItem);
        }

    }

    //empty
    if(ui->nodeList->count()==0) {
        QListWidgetItem *newItem = new QListWidgetItem("**EMPTY**");
        ui->nodeList->insertItem(0,newItem);
    }

    QString lscommand="ls /local_scratch/slee91";

    myProcess = new QProcess(this);
    myProcess->start(lscommand); //start the process

    myProcess->waitForFinished();

    result = QString(myProcess->readAll());

    processed = result.split("\n");

    //clear the listView
    ui->outList->clear();

    //add to listview
    for(iter = processed.constBegin(); iter != processed.constEnd(); iter++) {

        line=(*iter).toLocal8Bit().constData();
        if(line.contains(".root")||line.contains(".hdr")||line.contains(".sln")||line.contains(".dat")) {
            QListWidgetItem *newItem = new QListWidgetItem(line);
            ui->outList->insertItem(0,newItem);
        }


    }


}



void MainWindow::on_refreshButton_clicked()
{
    updateView(ui->nodeCombo->currentText());
}

void MainWindow::on_killAllButton_clicked()
{

    //run command
    QProcess *process = new QProcess(this);
    QString command="ka";
    QString host = ui->nodeCombo->currentText();

    process->start(command.arg(host));
    process->waitForFinished();
    updateView(host);
}

void MainWindow::updateOverview() {
    QString command = "ha";
    QProcess *process = new QProcess(this);
    connect(process,SIGNAL(finished(int)),this, SLOT(overviewFinished()));
    process->start(command);
}

void MainWindow::overviewFinished() {

}

void MainWindow::on_overviewRefresh_clicked()
{
    updateOverview();
}

void MainWindow::on_pushButton_clicked()
{

    //select scriptFile;
    QString input = QFileDialog::getOpenFileName(this,"Please enter the script file name to run", DEFAULT_LOCATION);
    if(input==NULL) return; //canceled

    //check if the script is valid.
    TimeInfo info = utility->inspect(input);
    if(info.valid==false) {
        QMessageBox msgBox;
        msgBox.setText("Invalid script file.");
        msgBox.exec();
        return;
    }

    //valid!
    ui->txtScript->setText(input);

    //work out PWD
    int slashIndex = input.lastIndexOf("/");
    ui->txtPWD->setText(input.left(slashIndex));

    //parsed time info
    ui->txtNOP->setText(QString::number(info.nop,10));
    ui->txtPerSlice->setText(QString::number(info.perslice));
    ui->txtTotalTime->setText(QString::number(info.totalTime));

    ui->btnSplit->setEnabled(true);


}

void MainWindow::on_btnSplit_clicked()
{
    //split!
    //prepare input
    JSInput input;
    input.inputFile = ui->txtScript->text();
    if(ui->comboCluster->currentText().compare("")==0) {
        input.numberOfCore = HYDRA_CORE;
    }else if(ui->comboCluster->currentText().compare("arc")==0) {
        input.numberOfCore = ARC_CORE;
    }
    input.numberOfProjections = ui->txtNOP->text().toInt();
    input.perProjectionSlice = ui->txtPerSlice->text().toFloat();
    input.PWD = ui->txtPWD->text();
    input.totalTime = ui->txtTotalTime->text().toInt();

    //output prefix
    QString temp = input.inputFile.left(input.inputFile.lastIndexOf(".mac"));
    QString outputTemplate("%1_cluster");
    input.outputPrefix = outputTemplate.arg(temp);
    JSOutput* output = utility->js(&input);

    //update information
    if(output->valid) {
        this->jsoutput = output;
        ui->txtRequiredMachines->setText(QString::number(this->jsoutput->batchFiles.count(),10));

    }
    ui->btnRun->setEnabled(true);
}

void MainWindow::on_comboCluster_currentIndexChanged(QString cluster)
{

   /* if(cluster.compare("hydra")==0) {
        //PER NODE TAB
        //initialize combo Box;
        QString base("hydra%1.eecs.utk.edu");
        ui->nodeCombo->clear();
        for(int i=0;i<HYDRA_NUMBER;i++) {
           ui->nodeCombo->insertItem(i,base.arg(i));
        }
   }else if(cluster.compare("arc")==0) {
       //PER NODE TAB
       //initialize combo Box;
       QString base("arc%1.eecs.utk.edu");
       ui->nodeCombo->clear();
       for(int i=0;i<ARC_NUMBER;i++) {
          ui->nodeCombo->insertItem(i,base.arg(i));
      }

   }*/

}

void MainWindow::on_btnRun_clicked()
{
    int startmachine;
    int requiredmachine;

    //check if jsoutput is ready.
    if(this->jsoutput==NULL) {
        QMessageBox::warning(this,"Simulation not ready","Please split the simulation first");
        return;
    }

    //check if all settings are entered
    startmachine = ui->txtStartMachine->text().toInt();
    requiredmachine = ui->txtRequiredMachines->text().toInt();

    //everything is ready! Let's run
    bool outcome = utility->runSimulation(jsoutput,ui->comboCluster->currentText(),startmachine,requiredmachine);
    QMessageBox::information(this,"Information","Job submitted successfully");
    return;
}


void MainWindow::on_killButton_clicked()
{
    QString where = this->ui->nodeCombo->currentText();
    QString what = this->ui->nodeList->currentItem()->text();
    QStringList temp = what.split(" ");

    QString pid = temp[3];

    utility->killIt(where,pid);
    this->updateView(ui->nodeCombo->currentText());
}

void MainWindow::on_btnrefresh_merge_clicked()
{
    //update merge view
    this->updateMergeView();
}

void MainWindow::updateMergeView() {

    return;
    QProcess** myProcess;
    QString result;
    QStringList processed;
    QString line;
    QStringList::const_iterator iter;

    int howMany;

    //clear the list
    ui->tableMerge->clear();
    ui->tableMerge->setColumnCount(3); //number, host, filename
    ui->tableMerge->setRowCount(0);
    ui->tableMerge->setColumnWidth(2,500);
    //gather output files from all directory.
    int from = ui->txtMergeRangeFrom->text().toInt();
    int to = ui->txtMergeRangeTo->text().toInt();
    howMany = to-from+1;
    if(howMany<1) return;

    //run simulations
    myProcess = new QProcess*[howMany];

    for(int i=0; i<howMany;i++) {
        QString lscommand="ssh hydra%1 ls %2";
        int whichMachine = from + i;
        lscommand = lscommand.arg(QString::number(whichMachine,10),QString(OUTPUT_DIRECTORY));
        myProcess[i] = new QProcess(this);
        myProcess[i]->start(lscommand); //start the process
    }


    //gather the output
    QString machineNameString("hydra%1");
    int fileCount=0;
    int newRowCount;
    for(int i=0;i<howMany;i++) {

        myProcess[i]->waitForFinished();
        result = QString(myProcess[i]->readAll());
        processed = result.split("\n");

          int totalCount=0;
          for(iter = processed.constBegin(); iter != processed.constEnd(); iter++) {
              line=(*iter).toLocal8Bit().constData();
              if(line.contains(".root")||line.contains(".hdr")||line.contains(".sln")||line.contains(".dat")
                  ||line.contains(".txt")) {
                totalCount++;
              }
          }
       newRowCount = totalCount+(ui->tableMerge->rowCount());
       ui->tableMerge->setRowCount(newRowCount);

        //add to listview

        for(iter = processed.constBegin(); iter != processed.constEnd(); iter++) {
            line=(*iter).toLocal8Bit().constData();
            if(line.contains(".root")||line.contains(".hdr")||line.contains(".sln")||line.contains(".dat")
                ||line.contains(".txt")) {

                QTableWidgetItem *fileName = new QTableWidgetItem(line);

                //file number
                QTableWidgetItem *totalNumber = new QTableWidgetItem(QString::number(fileCount,10));
                QTableWidgetItem *machineName = new QTableWidgetItem(machineNameString.arg(i+from));

                ui->tableMerge->setItem(fileCount,0,totalNumber);
                ui->tableMerge->setItem(fileCount,1,machineName);
                ui->tableMerge->setItem(fileCount,2,fileName);
                fileCount++;
            }
        }



    }


}



void MainWindow::on_btnfilter_clicked()
{
    filterMergeView(ui->txtFilter->text());
}

void MainWindow::filterMergeView(QString filter) {
    int totalRow;

    totalRow = ui->tableMerge->rowCount();
    for(int i=0;i<totalRow;i++) {
        QTableWidgetItem* widget = ui->tableMerge->item(i,2);
        if(widget==NULL) continue;
        QString fileName = widget->text();
        if(fileName.contains(filter)==false) {
            //contains
            ui->tableMerge->removeRow(i);
            totalRow=totalRow-1;
            i=i-1;
            continue;
        }
    }
}

void MainWindow::on_btnClean_clicked() {

}


void MainWindow::on_btnmerge_clicked()
{
 mergeOutput();
}

void MainWindow::mergeOutput() {
    //copy all the files to here.

}

void MainWindow::on_btnExplore_clicked()
{
    QStringList inputFileNames = QFileDialog::getOpenFileNames(this,"Please enter the file names to convert","/local_scratch/slee91","Gate ASCII files(*Singles.dat)");

   ui->listProjectionInput->clear(); //clear the list

   //add the item to the list
   for(int i=0;i<inputFileNames.size();i++) {
       QListWidgetItem* item = new QListWidgetItem(inputFileNames[i]);
       ui->listProjectionInput->addItem(item);
   }

}

void MainWindow::convert(QString inputFileName,QString outputFileName) {
    //open file
    QFile inputFile(inputFileName);
    if(inputFile.open(QFile::ReadOnly)==false) {
        QMessageBox::warning(this,"Input error","could not open input file");
        return;
    }
    QTextStream instream(&inputFile);

    //write file
    QFile outputFile(outputFileName);
    if(outputFile.open(QFile::WriteOnly|QFile::Truncate)==false) {
        QMessageBox::warning(this,"Output error","could not open output file");
        return;
    }
    QDataStream outstream(&outputFile);
    outstream.setByteOrder(QDataStream::LittleEndian);

    int headID;
    int crystalID;
    int pixelID;
    int comptonScatter;

    quint16 data[NUMBER_OF_HEAD][NUMBER_OF_PMT][PMT_HEIGHT][PMT_WIDTH]={0};

    //work out positions
    int headIDIndex = ui->txtheadid->text().toInt();
    int crystalIDIndex = ui->txtcrystalid->text().toInt();
    int pixelIDIndex = ui->txtpixelid->text().toInt();
    int comptonScatterPhantomIndex = ui->txtcomptonphantom->text().toInt();
    int comptonScatterDetectorIndex = ui->txtcomptondetector->text().toInt();
    
    //576*9
    while(instream.atEnd()==false) {

        QString line = instream.readLine();
        QStringList list = line.split(' ',QString::SkipEmptyParts);
        if(list.count() <= comptonScatterDetectorIndex) continue; //input is corrupted so
        //format
        //headID = 7th (index 6)
        //crystal ID = 8th (7)
        //pixel ID  = 9th (8)
        //compton scatter(phatom,detector) = 15th,16th (14,15)
        //energy = 11th (10)
        headID = list[headIDIndex].toInt();
        crystalID = list[crystalIDIndex].toInt();
        pixelID = list[pixelIDIndex].toInt();

        /* if(ui->cbCompton->checkState()==Qt::Unchecked) {//when not to include compton scatter
            comptonScatter = list[comptonScatterPhantomIndex].toInt() + list[comptonScatterDetectorIndex].toInt();

            //do not include compton scatter
            if(comptonScatter>0) {//this is a scattered photon
               continue;
            }

        }*/

        int pixel_column = pixelID % (PMT_WIDTH);
        int pixel_row = pixelID / (PMT_WIDTH);
        data[headID][crystalID][pixel_row][pixel_column]++;
    }

    //data[NUMBER_OF_HEAD][NUMBER_OF_PMT][PMT_HEIGHT][PMT_WIDTH]={
    //convert 3x3 to a single image
    for(int i=0;i<NUMBER_OF_HEAD;i++) {
        //0 1 2
        for(int z=0;z<PMT_HEIGHT;z++) {
         for(int j=0;j<=2;j++) { //across number of pmt 3
             for(int k=0;k<PMT_WIDTH;k++) {
               outstream<<data[i][j][z][k];
             }
         }
        }
        //3 4 5
        for(int z=0;z<PMT_HEIGHT;z++) {
         for(int j=3;j<=5;j++) { //across number of pmt 3
             for(int k=0;k<PMT_WIDTH;k++) {
               outstream<<data[i][j][z][k];
             }
         }
        }
        //6 7 8
        for(int z=0;z<PMT_HEIGHT;z++) {
         for(int j=6;j<=8;j++) { //across number of pmt 3
             for(int k=0;k<PMT_WIDTH;k++) {
               outstream<<data[i][j][z][k];
             }
         }
        }

    }



}

void MainWindow::on_btnConvert_clicked()
{
    //iterete through all projection images
    int count = ui->listProjectionInput->count();
    for(int i=0;i<count;i++) {
        QString filename =ui->listProjectionInput->item(i)->text();
        convert(filename,filename+".proj");

    }
}

void MainWindow::on_cbCompton_clicked()
{

}

void MainWindow::on_cbCompton_toggled(bool checked)
{
    this->ui->txtcomptondetector->setEnabled(checked);
    this->ui->txtcomptonphantom->setEnabled(checked);
}

void MainWindow::on_btnExplore_2_clicked()
{

}

void MainWindow::on_btnAnalyzeInputExplore_clicked()
{
    QString inputFileName = QFileDialog::getOpenFileName(this,"Please enter the file name to convert","/local_scratch/slee91");
    if(inputFileName==NULL) return; //canceled

    ui->txtAnalyzeInput->setText(inputFileName); //input file name
    ui->txtAnalyzeOutput->setText(inputFileName+".dat"); //output file name


}

void MainWindow::on_btnAnalyzeOutputExplore_clicked()
{
    QString outputFileName= QFileDialog::getSaveFileName(this,"Please enter the file name to convert","/local_scratch/slee91");
    if(outputFileName==NULL) return; //canceled

    ui->txtAnalyzeOutput->setText(outputFileName); //output file name
}

void MainWindow::on_btnEnergySpectrum_clicked()
{
    //generate energy specturm
    generateEnergySpectrum(ui->txtAnalyzeInput->text(), ui->txtAnalyzeOutput->text(), ui->txtAnalyzeOutput->text()+".hist");

}

void MainWindow::generateEnergySpectrum(QString inputFileName,QString outputFileName, QString spectrumFileName) {
    //int32
    const int HISTOGRAM_SIZE=1025;
    int histogram[HISTOGRAM_SIZE]={0};

    //open file
    QFile inputFile(inputFileName);
    if(inputFile.open(QFile::ReadOnly)==false) {
        QMessageBox::warning(this,"Input error","could not open input file");
        return;
    }
    QTextStream instream(&inputFile);

    //write file
    QFile outputFile(outputFileName);
    if(outputFile.open(QFile::WriteOnly|QFile::Truncate)==false) {
        QMessageBox::warning(this,"Output error","could not open output file");
        return;
    }
    QDataStream outstream(&outputFile);
    outstream.setByteOrder(QDataStream::LittleEndian);

    //energy spectrum
    QFile kevFile(spectrumFileName);
    if(kevFile.open(QFile::WriteOnly|QFile::Truncate)==false) {
        QMessageBox::warning(this,"Output error","could not open output file");
        return;
    }
    QTextStream kevstream(&kevFile);

    float energy;

    quint16 data[NUMBER_OF_HEAD][NUMBER_OF_PMT][PMT_HEIGHT][PMT_WIDTH]={0};

    //work out positions
    //energy spectrum
    int energyIndex = ui->txtEnergyIndex->text().toInt();
    int comptonDetectorIndex = ui->txtcomptondetector->text().toInt();

    int histogramIndex;
    int comptonDetector;
    int comptonPhantom;

    //576*9
    while(instream.atEnd()==false) {
        QString line = instream.readLine();
        QStringList list = line.split(' ',QString::SkipEmptyParts);
        //format
        //headID = 7th (index 6)
        //crystal ID = 8th (7)
        //pixel ID  = 9th (8)
        //compton scatter(phatom,detector) = 15th,16th (14,15)
        //energy = 11th (10)

        energy = list[energyIndex].toFloat()*1000; //convert to keV
        comptonDetector = list[comptonDetectorIndex].toInt(); //compton Detector

        //skip compton detector?
        if(ui->cb_compton_detector->checkState() == Qt::Unchecked) {
            //not checked so do not include
            if(comptonDetector!=0) continue; //skip compton scatter photons
        }
        //write to output file.
        outstream<<energy<<endl;


        if(ui->radioCeil->isChecked()) {
            //ceiling
            histogramIndex = ceil(energy);
        }else if(ui->radioFloor->isChecked()) {
            //flooring
            histogramIndex = floor(energy);
        }else {
            //round
            histogramIndex = ceil(energy+0.5);
        }    
        histogram[histogramIndex]++; //add to appropriate bin
    }

    //write histogram!
    for(int i=0;i<HISTOGRAM_SIZE;i++) {
        kevstream<<i<<" "<<histogram[i]<<endl;
    }

    //close files
    kevFile.close();
    outputFile.close();
}

void MainWindow::on_pushButton_2_clicked()
{
    QStringList inputFileNames = QFileDialog::getOpenFileNames(this,"Please enter the file names to convert","/local_scratch/slee91","Projection file(*.proj *.sin)");

   ui->listMergeProjection->clear(); //clear the list

   //add the item to the list
   for(int i=0;i<inputFileNames.size();i++) {
       QListWidgetItem* item = new QListWidgetItem(inputFileNames[i]);
       ui->listMergeProjection->addItem(item);
   }

   //update number of projections
   ui->txtMergeProjection_nop->setText(QString::number(inputFileNames.size(),10));
}

void MainWindow::on_btn_mergeprojection_explore_clicked()
{
    QString outputFileName= QFileDialog::getSaveFileName(this,"Please enter the output file name","/local_scratch/slee91");
    if(outputFileName==NULL) return; //canceled

    ui->txtMergeProjection_output->setText(outputFileName); //output file name

}

void MainWindow::on_pushButton_3_clicked()
{
    mergeProjectionToInveon();
}

void MainWindow::mergeProjectionToInveon() {
      //read 72x72x16bit unsigned integer (little-endian)
       //write to 68x68x32bit unsigned integer (little-endian)
       QString filename;
       quint16 unsigned16;
       quint32 unsigned32;

       vector<qint32> *detector1_projection = new vector<qint32>();
       vector<qint32> *detector2_projection = new vector<qint32>();
       qint32 detector1_total=0;
       qint32 detector2_total=0;

       QString outputStat("%1.stat");

       const int inputDimensionX= 72;
       const int inputDimensionY= 72;
       const int outputDimensionX=68;
       const int outputDimensionY=68;
       const int difX = (inputDimensionX - outputDimensionX)/2;
       const int difY = (inputDimensionY - outputDimensionY)/2;

       QDataStream out;
       QTextStream outStat;

       //open the outputfile & statfile
       QFile* outputFile = new QFile(ui->txtMergeProjection_output->text());
       QFile* outputStatFile = new QFile(outputStat.arg(ui->txtMergeProjection_output->text()));

       outputFile->open(QIODevice::WriteOnly|QIODevice::Truncate);
       outputStatFile->open(QIODevice::WriteOnly|QIODevice::Truncate);

       out.setDevice(outputFile);
       out.setByteOrder(QDataStream::LittleEndian);
       outStat.setDevice(outputStatFile);

       //open input file and process it one at a time
       for(int i=0;i<ui->listMergeProjection->count();i++) {
           //read one file at a time
          filename = ui->listMergeProjection->item(i)->text();
          QFile file(filename);
          file.open(QIODevice::ReadOnly);
          QDataStream in(&file);
          in.setByteOrder(QDataStream::LittleEndian);
          quint32 projection_sum=0;
          for(int j=0;j<inputDimensionX;j++) {
              for(int z=0;z<inputDimensionY;z++) {
                  in>>unsigned16;
                  //get rid of the first and last two pixel rows
                  if(j<difY||j>=inputDimensionY-difY)
                   continue;
                  //get rid of the first and last two pixel columns
                  //0,1 66 67
                  if(z<difX||z>=inputDimensionX-difX)
                    continue;

                  //otherwise write to a file;
                  unsigned32 = (quint32)unsigned16;
                  out<<unsigned32;

                  //sum up all counts
                  projection_sum += (qint32)unsigned32;

              }

          }
          //put it into the table
          detector1_projection->push_back(projection_sum);
          detector1_total+=(quint64)projection_sum;

       }
       outputFile->close();

       //write the other half
       //open the outputfile
       outputFile = new QFile(ui->txtMergeProjection_output->text());
       outputFile->open(QIODevice::WriteOnly|QIODevice::Append);

       out.setDevice(outputFile);
       out.setByteOrder(QDataStream::LittleEndian);

       //open input file and process it one at a time
       for(int i=0;i<ui->listMergeProjection->count();i++) {
           //read one file at a time
          filename = ui->listMergeProjection->item(i)->text();
          QFile file(filename);
          file.open(QIODevice::ReadOnly);

          QDataStream in(&file);
           in.setByteOrder(QDataStream::LittleEndian);
          for(int j=0;j<inputDimensionX;j++) {
              for(int z=0;z<inputDimensionY;z++) {
               //skip the first projections
               in>>unsigned16;
              }
          }

          quint32 projection_sum=0;

          for(int j=0;j<inputDimensionX;j++) {
              for(int z=0;z<inputDimensionY;z++) {
                  in>>unsigned16;
                  //get rid of the first and last two pixel rows
                  if(j<difY||j>=inputDimensionY-difY)
                   continue;
                  //get rid of the first and last two pixel columns
                  //0,1 66 67
                  if(z<difX||z>=inputDimensionX-difX)
                    continue;

                  //otherwise write to a file;
                  unsigned32 = (quint32)unsigned16;
                  out<<unsigned32;
                  projection_sum+=unsigned32;


              }

          }
          //put it into the table
          detector2_projection->push_back(projection_sum);
          detector2_total+=(quint64)projection_sum;

       }
       outputFile->close();
       QMessageBox::information(this,"Conversion complete!", "Data is successfully written to "+ui->txtMergeProjection_output->text());

       outStat<<"Total events seen by Detector 1"<<(qint32)detector1_total<<endl;
       outStat<<detector1_projection->size()<<" projections from Detector 1"<<endl;

       vector<qint32>::iterator itor;
       for(itor=detector1_projection->begin();itor!=detector1_projection->end();itor++) {
           outStat<<(qint32)(*itor)<<endl;
       }
       outStat<<"Total events seen by Detector 2"<<(qint32)detector2_total<<endl;
       outStat<<detector2_projection->size()<<" projections from Detector 2"<<endl;
       for(itor=detector2_projection->begin();itor!=detector2_projection->end();itor++) {
           outStat<<(qint32)(*itor)<<endl;
       }
       outputStatFile->close();
}

void MainWindow::on_btnEnergySpectrum_2_clicked()
{
    //process Scatter Fraction information
    processScatterFraction(ui->txtAnalyzeInput->text());
}

void MainWindow::processScatterFraction(QString inputFileName) {
    //open file
    QFile inputFile(inputFileName);
    if(inputFile.open(QFile::ReadOnly)==false) {
        QMessageBox::warning(this,"Input error","could not open input file");
        return;
    }
    QTextStream instream(&inputFile);

    //work out positions
    int comptonDetectorIndex = ui->txtComptonDetector->text().toInt();
    int comptonPhantomIndex = ui->txtComptonPhantom->text().toInt();

    long total=0;
    long phantom_scattered=0;
    long detector_scattered=0;
    long both_scattered=0;
    long or_scattered=0;

    int comptonDetector;
    int comptonPhantom;
    //576*9

    while(instream.atEnd()==false) {
        QString line = instream.readLine();
        QStringList list = line.split(' ',QString::SkipEmptyParts);
        //format
        //headID = 7th (index 6)
        //crystal ID = 8th (7)
        //pixel ID  = 9th (8)
        //compton scatter(phatom,detector) = 15th,16th (14,15)
        //energy = 11th (10)

        comptonDetector = list[comptonDetectorIndex].toInt(); //compton Detector
        comptonPhantom = list[comptonPhantomIndex].toInt(); //compton Phantom


        if(comptonPhantom>0) phantom_scattered++;
        if(comptonDetector>0) detector_scattered++;
        if(comptonPhantom>0&&comptonDetector>0) both_scattered++;
        if(comptonPhantom>0||comptonDetector>0) or_scattered++;
        total++; //add to total.

    }
    inputFile.close();
    QString outputStringTemplate("total = %1\nphantom scatter = %2\ndetector scatter = %3\nboth scatter = %4\nat least one scatter = %5\n");
    ui->txtScatterFraction_output->setText(outputStringTemplate.arg(QString::number(total,10),
                                                                    QString::number(phantom_scattered,10),
                                                                    QString::number(detector_scattered,10),
                                                                    QString::number(both_scattered,10),
                                           QString::number(or_scattered,10)));
}


void MainWindow::on_btnsplit2_clicked()
{
    //prepare input
    JSInput input;
    input.inputFile = ui->txtScript->text();
    if(ui->comboCluster->currentText().compare("hydra")==0) {
        input.numberOfCore = HYDRA_CORE;
    }else if(ui->comboCluster->currentText().compare("arc")==0) {
        input.numberOfCore = ARC_CORE;
    }
    input.numberOfProjections = ui->txtNOP->text().toInt();
    input.perProjectionSlice = ui->txtPerSlice->text().toFloat();
    input.PWD = ui->txtPWD->text();
    input.totalTime = ui->txtTotalTime->text().toInt();

    //output prefix
    QString temp = input.inputFile.left(input.inputFile.lastIndexOf(".mac"));
    QString outputTemplate("%1_cluster");
    input.outputPrefix = outputTemplate.arg(temp);
    JSOutput* output = utility->jsForCluster(&input);

    //update information
    if(output->valid) {
        this->jsoutput = output;
        ui->txtRequiredMachines->setText(QString::number(this->jsoutput->batchFiles.count(),10));

    }
    ui->btnRun->setEnabled(true);
}
