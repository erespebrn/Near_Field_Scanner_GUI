#include "nf_plot.h"
#include "ui_nf_plot.h"

NF_plot::NF_plot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NF_plot)
{
    ui->setupUi(this);
    ui->nearfield_plot->setVisible(false);
    ui->maxhold_plot->setVisible(false);

    ui->save_selected_button->setVisible(false);
    ui->cancel_saving_button->setVisible(false);

    ui->export_plots_button->setVisible(false);
    ui->interpolate_radio->setVisible(false);

    ui->export_plots_button->setEnabled(false);
    ui->custom_radioButton->setVisible(false);
    ui->default_radioButton->setVisible(false);

    this->setWindowTitle("Near-Field Plotting Tool");

    qDebug() << "NF_plot Created";
}

NF_plot::~NF_plot()
{
    delete ui;
    qDebug() << "NF_plot Destroyed";
}


void NF_plot::add_bg_image(QImage *im)
{
    pcb_img = im;
    pcb_img_cpy = *im;
}

void NF_plot::assign_field_comp(bool xc, bool yc)
{
    x_comp = xc;
    y_comp = yc;
}

void NF_plot::maxhold_init(const size_t &sp)
{
    ui->maxhold_plot->legend->setVisible(true);
    ui->maxhold_plot->setInteractions(QCP::iSelectPlottables);
    ui->maxhold_plot->legend->setFont(QFont("Helvetica", 9));

    QPen pen;
    ui->maxhold_plot->addGraph();
    pen.setColor(Qt::yellow);
    ui->maxhold_plot->graph(0)->setPen(pen);
    ui->maxhold_plot->graph(0)->setLineStyle((QCPGraph::LineStyle)1);
    ui->maxhold_plot->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->maxhold_plot->graph(0)->setName("Max hold");
    ui->maxhold_plot->setBackground(Qt::black);
    ui->maxhold_plot->yAxis->setRange(low_lim, high_lim);

    ui->maxhold_plot->xAxis->setTicks(true);
    ui->maxhold_plot->yAxis->setTicks(true);
    ui->maxhold_plot->xAxis->setTickLabels(true);
    ui->maxhold_plot->yAxis->setTickLabels(true);

    ui->maxhold_plot->axisRect()->setupFullAxesBox();
    ui->maxhold_plot->graph(0)->setSelectable(QCP::SelectionType::stSingleData);

    ui->maxhold_plot->yAxis->setTickLabelColor(Qt::lightGray);
    ui->maxhold_plot->yAxis->setBasePen(QPen(Qt::lightGray));
    ui->maxhold_plot->yAxis->setLabelColor(Qt::lightGray);
    ui->maxhold_plot->yAxis->setTickPen(QPen(Qt::lightGray));
    ui->maxhold_plot->yAxis->setSubTickPen(QPen(Qt::lightGray));

    ui->maxhold_plot->xAxis->setTickLabelColor(Qt::lightGray);
    ui->maxhold_plot->xAxis->setBasePen(QPen(Qt::lightGray));
    ui->maxhold_plot->xAxis->setLabelColor(Qt::lightGray);
    ui->maxhold_plot->xAxis->setTickPen(QPen(Qt::lightGray));
    ui->maxhold_plot->xAxis->setSubTickPen(QPen(Qt::lightGray));

    ui->maxhold_plot->axisRect(0)->setMinimumSize(600,ui->nearfield_plot->minimumHeight()-50);
    ui->maxhold_plot->axisRect(0)->setMaximumSize(600,ui->nearfield_plot->maximumHeight()-50);

    ui->maxhold_plot->setMinimumSize(670, ui->nearfield_plot->minimumHeight());
    ui->maxhold_plot->setMaximumSize(670, ui->nearfield_plot->minimumHeight());

    //ui->maxhold_plot->setFixedSize(ui->maxhold_plot->size());
    connect(ui->maxhold_plot, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));

    tracer = new QCPItemTracer(ui->maxhold_plot);
    dwPoints = new QCPGraph(ui->maxhold_plot->xAxis, ui->maxhold_plot->yAxis);
    dwPoints->setAdaptiveSampling(false);
    dwPoints->setLineStyle(QCPGraph::lsNone);
    dwPoints->setScatterStyle(QCPScatterStyle::ssCross);
    dwPoints->setPen(QPen(QBrush(Qt::red), 2));
    ui->maxhold_plot->graph(1)->setName("Selected Point");
    std::vector<double> temp(sp, -200.0);
    maxh = maxh.fromStdVector(temp);

    this->adjustSize();

    this->setFixedSize(this->size());
}

void NF_plot::plot_maxhold()
{
    x.clear();
    y.clear();
    for(size_t i=0; i<freq->size(); i++){
        x.push_back((double)freq->at(i));
        y.push_back((double)mag->at(i));
    }
    for(size_t i=0; i<(size_t)y.size(); i++){
        if(y[i] > maxh[i])
            maxh[i] = y[i];
    }

    ui->maxhold_plot->graph(0)->setData(x, maxh);
    ui->maxhold_plot->graph(0)->rescaleAxes(true);
    this->add_single_marker(fr);
    ui->maxhold_plot->replot();
}

void NF_plot::add_single_marker(int dIndex)
{
    if(!multiple_export_time){
        if(m_selected){
            QVector<double> values, keys;

            tracer->setGraph(ui->maxhold_plot->graph(0));
            tracer->setInterpolating(true);
            tracer->setVisible(false);

            tracer->setGraphKey(x[dIndex]);
            tracer->updatePosition();
            keys.push_back(x[dIndex]);
            values.push_back(tracer->position->coords().y());

            dwPoints->data()->clear();
            dwPoints->addData(keys, values);
        }
    }else{
        if(multiple_export_time){
            QVector<double> values, keys;

            tracer->setGraph(ui->maxhold_plot->graph(0));
            tracer->setInterpolating(true);
            tracer->setVisible(false);

            tracer->setGraphKey(x[dIndex]);
            tracer->updatePosition();
            keys.push_back(x[dIndex]);
            values.push_back(tracer->position->coords().y());
            dwPoints->addData(keys, values);
        }
    }
    ui->maxhold_plot->replot();
}

void NF_plot::onColorRangeChanged(QCPRange range)
{
    Q_UNUSED(range);
}

void NF_plot::assign_data_tensors(std::vector<float> *f, std::vector<float> *m,
                                  std::vector<std::vector<float>> *t2d,
                                  std::vector<std::vector<std::vector<float>>> *dv,
                                  std::vector<std::vector<std::vector<float>>> *dv_t,
                                  std::vector<float>* ph,
                                  std::vector<std::vector<float>>* t2d_ph,
                                  std::vector<std::vector<std::vector<float>>>*dv_ph,
                                  std::vector<std::vector<std::vector<float>>>*dv_t_ph)
{
    freq = f;
    mag = m;
    temp2d_mag = t2d;
    scan_data_mag = dv;
    scan_data_t_mag = dv_t;
    phase = ph;
    temp2d_phase = t2d_ph;
    scan_data_phase = dv_ph;
    scan_data_t_phase = dv_t_ph;
}

void NF_plot::allow_exports(bool allow)
{
    ui->export_plots_button->setEnabled(allow);
}

void NF_plot::on_export_plots_button_clicked()
{
    dwPoints->data()->clear();
    ui->maxhold_plot->replot();
    ui->export_plots_button->setEnabled(false);
    ui->save_selected_button->setVisible(true);
    ui->cancel_saving_button->setVisible(true);
    ui->custom_radioButton->setVisible(true);
    ui->default_radioButton->setVisible(true);
    multiple_export_time = true;
    QMessageBox::information(this, "Export plots", "Select the frequencies to export and press Save to export");
}

void NF_plot::on_cancel_saving_button_clicked()
{
    dwPoints->data()->clear();
    ui->maxhold_plot->replot();
    exp_points.clear();
    ui->export_plots_button->setEnabled(true);
    ui->save_selected_button->setVisible(false);
    ui->cancel_saving_button->setVisible(false);
    ui->custom_radioButton->setVisible(false);
    ui->default_radioButton->setVisible(false);
}



void NF_plot::static_maxhold_plot()
{
    x.clear();
    y.clear();
    for(size_t i=0; i<st_freq.size(); i++){
        x.push_back((double)st_freq.at(i));
    }
    ui->maxhold_plot->graph(0)->setData(x, maxh);
    ui->maxhold_plot->graph(0)->rescaleAxes(true);

    ui->maxhold_plot->replot();
    this->allow_exports(true);
}


void NF_plot::static_data_vectors_init()
{
    freq = &st_freq;

    mag = &st_mag;
    temp2d_mag = &st_temp2d_mag;
    scan_data_mag = &st_scan_data_mag;

    phase = &st_phase;
    temp2d_phase = &st_temp2d_phase;
    scan_data_phase = &st_scan_data_phase;

    scan_data_t_mag = &st_scan_data_t_mag;
    scan_data_t_phase = &st_scan_data_t_phase;

}





void NF_plot::display_first_freq(float f1, float f2)
{
    ui->maxhold_plot->xAxis->setRange(QCPRange(f1-8e6, f2+8e6));
    float f1_ = f1/1e6;
    f1_ = std::ceil(f1_ * 100.0) / 100.0;
    float f2_ = f2/1e6;
    f2_ = std::ceil(f2_ * 100.0) / 100.0;
    QString message1 = QString("%1 MHz").arg(f1_);
    fr_label->setText("Frequency: "+message1);

    QString message2 = QString("%1 MHz").arg(f2_);
    QVector<double> ticks;
    double a = (f2-f1)/5;
    ticks << f1 << int(f1+a) << int(f1+2*a) << int(f1+3*a) << int(f1+4*a) << int(f1+5*a);
    qDebug() << ticks;
    QVector<QString> labels;
    a = (f2_-f1_)/5;
    labels << message1
           << QString("%1 MHz").arg(int(f1_+a))
           << QString("%1 MHz").arg(int(f1_+2*a))
           << QString("%1 MHz").arg(int(f1_+3*a))
           << QString("%1 MHz").arg(int(f1_+4*a))
           << message2;

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);
    ui->maxhold_plot->xAxis->setTicker(textTicker);
}

void NF_plot::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Near-field Plot" ,"Are you sure you want to close?\n All plot data will be lost.",
                                                              QMessageBox::No | QMessageBox::Yes);
    if(resBtn != QMessageBox::Yes){
        event->ignore();
    }else{
        event->accept();
        emit plot_widget_closing();
    }
}

void NF_plot::on_default_radioButton_clicked(bool checked)
{
    multiple_export_time = checked;
    ui->custom_radioButton->setChecked(!checked);
}

void NF_plot::on_custom_radioButton_clicked(bool checked)
{
    multiple_export_time = !checked;
    ui->default_radioButton->setChecked(!checked);
}

