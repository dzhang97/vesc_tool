/*
    Copyright 2016 - 2017 Benjamin Vedder	benjamin@vedder.se
	Copyright 2017 Nico Ackermann	changed visualisation of throttle curve

    This file is part of VESC Tool.

    VESC Tool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VESC Tool is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#include "pageappppm.h"
#include "ui_pageappppm.h"
#include "util.h"

PageAppPpm::PageAppPpm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageAppPpm)
{
    ui->setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    mVesc = 0;

    ui->throttlePlot->addGraph();
    ui->throttlePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->throttlePlot->xAxis->setRange(-100,100);
    ui->throttlePlot->xAxis->grid()->setSubGridVisible(true);
    ui->throttlePlot->yAxis->setRange(-100,100);
    ui->throttlePlot->yAxis->grid()->setSubGridVisible(true);
    ui->throttlePlot->graph()->setName("Throttle Curve");
    ui->throttlePlot->xAxis->setLabel("Throttle Value in %");
    ui->throttlePlot->yAxis->setLabel("Output Value in %");
    ui->throttlePlot->legend->setVisible(true);
    ui->throttlePlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
}

PageAppPpm::~PageAppPpm()
{
    delete ui;
}

VescInterface *PageAppPpm::vesc() const
{
    return mVesc;
}

void PageAppPpm::setVesc(VescInterface *vesc)
{
    mVesc = vesc;

    if (mVesc) {
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.ctrl_type");
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.median_filter");
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.safe_start");
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.pid_max_erpm");
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.ramp_time_pos");
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.ramp_time_neg");
        ui->generalTab->addRowSeparator(tr("Multiple VESCs over CAN-bus"));
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.multi_esc");
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.tc");
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.tc_offset");
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.tc_max_diff");
        ui->generalTab->addRowSeparator(tr("Cruise Control via Slave"));
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.cruise_left");
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.cruise_right");
        ui->generalTab->addRowSeparator(tr("Reverse behavior for ""Current"" Control"));
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.max_erpm_for_dir_active");
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.max_erpm_for_dir");
        ui->generalTab->addRowSeparator(tr("Transmission"));
        ui->generalTab->addParamRow(mVesc->appConfig(), "app_transmission_conf.switch_erpm");

        ui->mappingTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.pulse_start");
        ui->mappingTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.pulse_end");
        ui->mappingTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.pulse_center");
        ui->mappingTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.hyst");

        ui->throttleCurveTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.throttle_exp");
        ui->throttleCurveTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.throttle_exp_brake");
        ui->throttleCurveTab->addParamRow(mVesc->appConfig(), "app_ppm_conf.throttle_exp_mode");

        ui->ppmMap->setVesc(mVesc);

        connect(mVesc->appConfig(), SIGNAL(paramChangedDouble(QObject*,QString,double)),
                this, SLOT(paramChangedDouble(QObject*,QString,double)));
        connect(mVesc->appConfig(), SIGNAL(paramChangedEnum(QObject*,QString,int)),
                this, SLOT(paramChangedEnum(QObject*,QString,int)));

        paramChangedEnum(0, "app_ppm_conf.throttle_exp_mode", 0);
    }
}

void PageAppPpm::paramChangedDouble(QObject *src, QString name, double newParam)
{
    (void)src;
    (void)newParam;

    if (name == "app_ppm_conf.throttle_exp" || name == "app_ppm_conf.throttle_exp_brake") {
        int mode = mVesc->appConfig()->getParamEnum("app_ppm_conf.throttle_exp_mode");
        float val_acc = mVesc->appConfig()->getParamDouble("app_ppm_conf.throttle_exp");
        float val_brake = mVesc->appConfig()->getParamDouble("app_ppm_conf.throttle_exp_brake");

        QVector<double> x;
        QVector<double> y;
        for (float i = -1.0;i < 1.0001;i += 0.002) {
            x.append(i * 100);
            double val = util::throttle_curve(i, val_acc, val_brake, mode) * 100;
            y.append(val);
        }

        ui->throttlePlot->graph()->setData(x, y);
        ui->throttlePlot->replot();
    }
}

void PageAppPpm::paramChangedEnum(QObject *src, QString name, int newParam)
{
    (void)src;
    (void)newParam;

    if (name == "app_ppm_conf.throttle_exp_mode") {
        paramChangedDouble(0, "app_ppm_conf.throttle_exp", 0.0);
    }
}
