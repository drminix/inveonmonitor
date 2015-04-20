#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include "gateutility.h"
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    void initialize();
    void updateView(QString string);
    void readFromStdout();
    void updateOverview();
    void overviewFinished();
    void updateMergeView();
    void filterMergeView(QString);
    void mergeOutput();
    void convert(QString,QString );
    void generateEnergySpectrum(QString, QString, QString);
    void mergeProjectionToInveon();
    void processScatterFraction(QString input);

private:
    Ui::MainWindow *ui;
    GateUtility* utility;
    JSOutput* jsoutput;

private slots:
    void on_btnsplit2_clicked();
    void on_btnEnergySpectrum_2_clicked();
    void on_pushButton_3_clicked();
    void on_btn_mergeprojection_explore_clicked();
    void on_pushButton_2_clicked();
    void on_btnEnergySpectrum_clicked();
    void on_btnAnalyzeOutputExplore_clicked();
    void on_btnAnalyzeInputExplore_clicked();
    void on_btnExplore_2_clicked();
    void on_cbCompton_toggled(bool checked);
    void on_cbCompton_clicked();
    void on_btnConvert_clicked();
    void on_btnExplore_clicked();
    void on_btnmerge_clicked();
    void on_btnfilter_clicked();
    void on_btnrefresh_merge_clicked();
    void on_killButton_clicked();
    void on_btnClean_clicked();
    void on_btnRun_clicked();
    void on_comboCluster_currentIndexChanged(QString );
    void on_btnSplit_clicked();
    void on_pushButton_clicked();
    void on_overviewRefresh_clicked();
    void on_killAllButton_clicked();
    void on_refreshButton_clicked();
    void on_nodeCombo_currentIndexChanged(QString );

};

#endif // MAINWINDOW_H
