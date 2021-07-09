#include "nf_plot_vna.h"
#include "ui_nf_plot.h"

NF_plot_vna::NF_plot_vna() : NF_plot()
{}

void NF_plot_vna::init_plot(const size_t x, const size_t y)
{
    x_points = x;
    y_points = y;

    ui->export_plots_button->setVisible(true);
    ui->interpolate_radio->setVisible(true);
    ui->image_preview->setVisible(false);
    ui->nearfield_plot->setVisible(true);
    ui->maxhold_plot->setVisible(true);

    ui->nearfield_plot->plotLayout()->clear();

    QPainter p;
    *pcb_img = pcb_img->convertToFormat(QImage::Format_ARGB32);
    p.begin(pcb_img);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(pcb_img->rect(), QColor(0, 0, 0, 100));
    p.end();

    ui->nearfield_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectOther);

    heatmap_x_rect_mag = new QCPAxisRect(ui->nearfield_plot);
    heatmap_x_rect_mag->setupFullAxesBox(true);
    heatmap_x_rect_phase = new QCPAxisRect(ui->nearfield_plot);
    heatmap_x_rect_phase->setupFullAxesBox(true);

    heatmap_y_rect_mag = new QCPAxisRect(ui->nearfield_plot);
    heatmap_y_rect_mag->setupFullAxesBox(true);
    heatmap_y_rect_phase = new QCPAxisRect(ui->nearfield_plot);
    heatmap_y_rect_phase->setupFullAxesBox(true);

    QCPTextElement *y_title_mag = new QCPTextElement(ui->nearfield_plot, "Y - Magnitude", QFont("sans", 10, QFont::Bold));
    QCPTextElement *y_title_phase = new QCPTextElement(ui->nearfield_plot, "Y - Phase", QFont("sans", 10, QFont::Bold));
    QCPTextElement *x_title_mag = new QCPTextElement(ui->nearfield_plot, "X - Magnitude", QFont("sans", 10, QFont::Bold));
    QCPTextElement *x_title_phase = new QCPTextElement(ui->nearfield_plot, "X - Phase", QFont("sans", 10, QFont::Bold));
    fr_label = new QCPTextElement(ui->nearfield_plot, "Frequency: 1.1MHz", QFont("sans", 12, QFont::Bold));

    foreach(QCPAxisRect *rect, ui->nearfield_plot->axisRects()){
      foreach(QCPAxis *axis, rect->axes()){
        axis->setLayer("axes");
        axis->grid()->setLayer("grid");
      }
    }

    //Colormap y-component magnitude
    colorMap_y_mag = new QCPColorMap(heatmap_y_rect_mag->axis(QCPAxis::atBottom), heatmap_y_rect_mag->axis(QCPAxis::atLeft));
    colorMap_y_mag->setInterpolate(false);
    colorMap_y_mag->setTightBoundary(true);
    colorMap_y_mag->data()->setSize(x_points, y_points);
    colorMap_y_mag->data()->setRange(QCPRange(0, x_points), QCPRange(0, y_points));

    //Colormap y-component phase
    colorMap_y_phase = new QCPColorMap(heatmap_y_rect_phase->axis(QCPAxis::atBottom), heatmap_y_rect_phase->axis(QCPAxis::atLeft));
    colorMap_y_phase->setInterpolate(false);
    colorMap_y_phase->setTightBoundary(true);
    colorMap_y_phase->data()->setSize(x_points, y_points);
    colorMap_y_phase->data()->setRange(QCPRange(0, x_points), QCPRange(0, y_points));

    //Colorscale y-component magnitude
    colorScale_y_mag = new QCPColorScale(ui->nearfield_plot);
    colorScale_y_mag->setType(QCPAxis::atRight);
    colorScale_y_mag->axis()->setLabel("dBuV");
    colorScale_y_mag->setMinimumSize(50, pcb_img->height()/2);
    colorScale_y_mag->setMaximumSize(50, pcb_img->height()/2);
    colorScale_y_mag->setBarWidth(10);

    //Colorscale y-component phase
    colorScale_y_phase = new QCPColorScale(ui->nearfield_plot);
    colorScale_y_phase->setType(QCPAxis::atRight);
    colorScale_y_phase->axis()->setLabel("deg");
    colorScale_y_phase->setMinimumSize(50, pcb_img->height()/2);
    colorScale_y_phase->setMaximumSize(50, pcb_img->height()/2);
    colorScale_y_phase->setBarWidth(10);

    QCPRange r_y_mag = QCPRange(-80, -40);
    QCPRange r_y_ph = QCPRange(-180, 180);
    colorScale_y_mag->setDataRange(r_y_mag);
    colorScale_y_phase->setDataRange(r_y_ph);

    colorMap_y_mag->setColorScale(colorScale_y_mag);
    colorMap_y_mag->setGradient(QCPColorGradient::gpJet);
    colorMap_y_mag->rescaleDataRange();

    QCPColorGradient grad_y(QCPColorGradient::gpHues);
    grad_y.setPeriodic(true);
    colorMap_y_phase->setColorScale(colorScale_y_phase);
    colorMap_y_phase->setGradient(grad_y);
    colorMap_y_phase->rescaleDataRange();

    //Colormap x-component magnitude
    colorMap_x_mag = new QCPColorMap(heatmap_x_rect_mag->axis(QCPAxis::atBottom), heatmap_x_rect_mag->axis(QCPAxis::atLeft));
    colorMap_x_mag->setInterpolate(false);
    colorMap_x_mag->setTightBoundary(true);
    colorMap_x_mag->data()->setSize(x_points, y_points);
    colorMap_x_mag->data()->setRange(QCPRange(0, x_points), QCPRange(0, y_points));

    //Colormap x-component phase
    colorMap_x_phase = new QCPColorMap(heatmap_x_rect_phase->axis(QCPAxis::atBottom), heatmap_x_rect_phase->axis(QCPAxis::atLeft));
    colorMap_x_phase->setInterpolate(false);
    colorMap_x_phase->setTightBoundary(true);
    colorMap_x_phase->data()->setSize(x_points, y_points);
    colorMap_x_phase->data()->setRange(QCPRange(0, x_points), QCPRange(0, y_points));

    //Colorscale x-component magnitude
    colorScale_x_mag = new QCPColorScale(ui->nearfield_plot);
    colorScale_x_mag->setType(QCPAxis::atRight);
    colorScale_x_mag->axis()->setLabel("dBuV");
    colorScale_x_mag->setMinimumSize(50, pcb_img->height()/2);
    colorScale_x_mag->setMaximumSize(50, pcb_img->height()/2);
    colorScale_x_mag->setBarWidth(10);

    //Colorscale x-component phase
    colorScale_x_phase = new QCPColorScale(ui->nearfield_plot);
    colorScale_x_phase->setType(QCPAxis::atRight);
    colorScale_x_phase->axis()->setLabel("deg");
    colorScale_x_phase->setMinimumSize(50, pcb_img->height()/2);
    colorScale_x_phase->setMaximumSize(50, pcb_img->height()/2);
    colorScale_x_phase->setBarWidth(10);

    QCPRange r_x_mag = QCPRange(-80, -40);
    QCPRange r_x_ph = QCPRange(-180, 180);
    colorScale_x_mag->setDataRange(r_x_mag);
    colorScale_x_phase->setDataRange(r_x_ph);

    colorMap_x_mag->setColorScale(colorScale_x_mag);
    colorMap_x_mag->setGradient(QCPColorGradient::gpJet);
    colorMap_x_mag->rescaleDataRange();

    QCPColorGradient grad_x(QCPColorGradient::gpHues);
    grad_x.setPeriodic(true);
    colorMap_x_phase->setColorScale(colorScale_x_phase);
    colorMap_x_phase->setGradient(grad_x);
    colorMap_x_phase->rescaleDataRange();

    //Assign elements to the plot layout
    ui->nearfield_plot->plotLayout()->addElement(0, 0, fr_label);
    ui->nearfield_plot->plotLayout()->addElement(1, 0, y_title_mag);
    ui->nearfield_plot->plotLayout()->addElement(2, 0, heatmap_y_rect_mag);
    ui->nearfield_plot->plotLayout()->addElement(2, 1, colorScale_y_mag);
    ui->nearfield_plot->plotLayout()->addElement(3, 0, x_title_mag);
    ui->nearfield_plot->plotLayout()->addElement(4, 0, heatmap_x_rect_mag);
    ui->nearfield_plot->plotLayout()->addElement(4, 1, colorScale_x_mag);
    ui->nearfield_plot->plotLayout()->addElement(1, 2, y_title_phase);
    ui->nearfield_plot->plotLayout()->addElement(2, 2, heatmap_y_rect_phase);
    ui->nearfield_plot->plotLayout()->addElement(2, 3, colorScale_y_phase);
    ui->nearfield_plot->plotLayout()->addElement(3, 2, x_title_phase);
    ui->nearfield_plot->plotLayout()->addElement(4, 2, heatmap_x_rect_phase);
    ui->nearfield_plot->plotLayout()->addElement(4, 3, colorScale_x_phase);


    //Background pixmap y-component magnitude
    image_pixmap_y_mag = new QCPItemPixmap(ui->nearfield_plot);
    image_pixmap_y_mag->setPixmap(QPixmap::fromImage(*pcb_img));
    image_pixmap_y_mag->setScaled(true, Qt::IgnoreAspectRatio);
    image_pixmap_y_mag->setClipAxisRect(qobject_cast<QCPAxisRect*>(ui->nearfield_plot->plotLayout()->element(2,0)));
    image_pixmap_y_mag->setClipToAxisRect(true);
    image_pixmap_y_mag->topLeft->setType(QCPItemPosition::PositionType::ptPlotCoords);
    image_pixmap_y_mag->bottomRight->setType(QCPItemPosition::PositionType::ptPlotCoords);
    foreach(QCPItemPosition *pos, image_pixmap_y_mag->positions()){
        pos->setAxes(heatmap_y_rect_mag->axis(QCPAxis::atBottom), heatmap_y_rect_mag->axis(QCPAxis::atLeft));
    }
    image_pixmap_y_mag->topLeft->setCoords(colorMap_y_mag->data()->keyRange().lower, colorMap_y_mag->data()->valueRange().upper);
    image_pixmap_y_mag->bottomRight->setCoords(colorMap_y_mag->data()->keyRange().upper, colorMap_y_mag->data()->valueRange().lower);
    ui->nearfield_plot->addLayer("image", ui->nearfield_plot->layer("main"), QCustomPlot::limAbove);
    image_pixmap_y_mag->setLayer("image");
    image_pixmap_y_mag->setVisible(true);


    //Background pixmap y-component phase
    image_pixmap_y_phase = new QCPItemPixmap(ui->nearfield_plot);
    image_pixmap_y_phase->setPixmap(QPixmap::fromImage(*pcb_img));
    image_pixmap_y_phase->setScaled(true, Qt::IgnoreAspectRatio);
    image_pixmap_y_phase->setClipAxisRect(qobject_cast<QCPAxisRect*>(ui->nearfield_plot->plotLayout()->element(2,2)));
    image_pixmap_y_phase->setClipToAxisRect(true);
    image_pixmap_y_phase->topLeft->setType(QCPItemPosition::PositionType::ptPlotCoords);
    image_pixmap_y_phase->bottomRight->setType(QCPItemPosition::PositionType::ptPlotCoords);
    foreach(QCPItemPosition *pos, image_pixmap_y_phase->positions()){
        pos->setAxes(heatmap_y_rect_phase->axis(QCPAxis::atBottom), heatmap_y_rect_phase->axis(QCPAxis::atLeft));
    }
    image_pixmap_y_phase->topLeft->setCoords(colorMap_y_phase->data()->keyRange().lower, colorMap_y_phase->data()->valueRange().upper);
    image_pixmap_y_phase->bottomRight->setCoords(colorMap_y_phase->data()->keyRange().upper, colorMap_y_phase->data()->valueRange().lower);
    ui->nearfield_plot->addLayer("image", ui->nearfield_plot->layer("main"), QCustomPlot::limAbove);
    image_pixmap_y_phase->setLayer("image");
    image_pixmap_y_phase->setVisible(true);

    //Background pixmap x-component magnitude
    image_pixmap_x_mag = new QCPItemPixmap(ui->nearfield_plot);
    image_pixmap_x_mag->setPixmap(QPixmap::fromImage(*pcb_img));
    image_pixmap_x_mag->setScaled(true, Qt::IgnoreAspectRatio);
    image_pixmap_x_mag->setClipAxisRect(qobject_cast<QCPAxisRect*>(ui->nearfield_plot->plotLayout()->element(4,0)));
    image_pixmap_x_mag->setClipToAxisRect(true);
    image_pixmap_x_mag->topLeft->setType(QCPItemPosition::PositionType::ptPlotCoords);
    image_pixmap_x_mag->bottomRight->setType(QCPItemPosition::PositionType::ptPlotCoords);
    foreach(QCPItemPosition *pos, image_pixmap_x_mag->positions()){
        pos->setAxes(heatmap_x_rect_mag->axis(QCPAxis::atBottom), heatmap_x_rect_mag->axis(QCPAxis::atLeft));
    }
    image_pixmap_x_mag->topLeft->setCoords(colorMap_x_mag->data()->keyRange().lower, colorMap_x_mag->data()->valueRange().upper);
    image_pixmap_x_mag->bottomRight->setCoords(colorMap_x_mag->data()->keyRange().upper, colorMap_x_mag->data()->valueRange().lower);
    image_pixmap_x_mag->setLayer("image");
    image_pixmap_x_mag->setVisible(true);

    //Background pixmap x-component phase
    image_pixmap_x_phase = new QCPItemPixmap(ui->nearfield_plot);
    image_pixmap_x_phase->setPixmap(QPixmap::fromImage(*pcb_img));
    image_pixmap_x_phase->setScaled(true, Qt::IgnoreAspectRatio);
    image_pixmap_x_phase->setClipAxisRect(qobject_cast<QCPAxisRect*>(ui->nearfield_plot->plotLayout()->element(4,2)));
    image_pixmap_x_phase->setClipToAxisRect(true);
    image_pixmap_x_phase->topLeft->setType(QCPItemPosition::PositionType::ptPlotCoords);
    image_pixmap_x_phase->bottomRight->setType(QCPItemPosition::PositionType::ptPlotCoords);
    foreach(QCPItemPosition *pos, image_pixmap_x_phase->positions()){
        pos->setAxes(heatmap_x_rect_phase->axis(QCPAxis::atBottom), heatmap_x_rect_phase->axis(QCPAxis::atLeft));
    }
    image_pixmap_x_phase->topLeft->setCoords(colorMap_x_phase->data()->keyRange().lower, colorMap_x_phase->data()->valueRange().upper);
    image_pixmap_x_phase->bottomRight->setCoords(colorMap_x_phase->data()->keyRange().upper, colorMap_x_phase->data()->valueRange().lower);
    image_pixmap_x_phase->setLayer("image");
    image_pixmap_x_phase->setVisible(true);

    //Margin group
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->nearfield_plot);
    colorScale_y_mag->setMarginGroup(QCP::msBottom|QCP::msRight, marginGroup);
    colorScale_x_mag->setMarginGroup(QCP::msBottom|QCP::msRight, marginGroup);

    ui->nearfield_plot->setMinimumSize(pcb_img->width()+colorScale_x_mag->maximumSize().width(), pcb_img->height()+70);
    ui->nearfield_plot->setMaximumSize(pcb_img->width()+colorScale_x_mag->maximumSize().width(), pcb_img->height()+70);

    heatmap_x_rect_mag->setMinimumSize(pcb_img->width()/2-colorScale_x_mag->maximumSize().width(), pcb_img->height()/2);
    heatmap_x_rect_mag->setMaximumSize(pcb_img->width()/2-colorScale_x_mag->maximumSize().width(), pcb_img->height()/2);
    heatmap_x_rect_phase->setMinimumSize(pcb_img->width()/2-colorScale_x_mag->maximumSize().width(), pcb_img->height()/2);
    heatmap_x_rect_phase->setMaximumSize(pcb_img->width()/2-colorScale_x_mag->maximumSize().width(), pcb_img->height()/2);

    heatmap_y_rect_mag->setMinimumSize(pcb_img->width()/2-colorScale_x_mag->maximumSize().width(), pcb_img->height()/2);
    heatmap_y_rect_mag->setMaximumSize(pcb_img->width()/2-colorScale_x_mag->maximumSize().width(), pcb_img->height()/2);
    heatmap_y_rect_phase->setMinimumSize(pcb_img->width()/2-colorScale_x_mag->maximumSize().width(), pcb_img->height()/2);
    heatmap_y_rect_phase->setMaximumSize(pcb_img->width()/2-colorScale_x_mag->maximumSize().width(), pcb_img->height()/2);

    connect(ui->nearfield_plot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
    connect(fr_label, SIGNAL(doubleClicked(QMouseEvent*)), this, SLOT(freq_labelDoubleClicked(QMouseEvent*)));

    ui->nearfield_plot->replot();
    ui->nearfield_plot->rescaleAxes();

    //Maxhold plot limits
    this->low_lim = -100.0;
    this->high_lim = 0.0;

    this->adjustSize();
}

void NF_plot_vna::add_data_point()
{
    std::vector<std::vector<std::vector<float>>> &data_mag = *scan_data_mag;
    std::vector<std::vector<std::vector<float>>> &data_phase = *scan_data_phase;
    std::vector<float> &m = *mag;
    std::vector<float> &p = *phase;

    if(c_scan_com == YCOMP){
        for( int g=0; g<ui->nearfield_plot->graphCount(); g++ )
            ui->nearfield_plot->graph(g)->data().data()->clear();

        colorMap_y_mag->data()->setCell(temp2d_mag->size()-1, y_points-data_mag.size()-1, m[fr]);
        colorMap_y_phase->data()->setCell(temp2d_phase->size()-1, y_points-data_phase.size()-1, p[fr]);
        ui->nearfield_plot->replot();
    }
    else if(c_scan_com == XCOMP){
        for( int g=0; g<ui->nearfield_plot->graphCount(); g++ )
            ui->nearfield_plot->graph(g)->data().data()->clear();

        colorMap_x_mag->data()->setCell(temp2d_mag->size()-1, y_points-data_mag.size()-1, m[fr]);
        colorMap_x_phase->data()->setCell(temp2d_phase->size()-1, y_points-data_phase.size()-1, p[fr]);
        ui->nearfield_plot->replot();
    }
}

void NF_plot_vna::replot_heatmap()
{
    std::vector<std::vector<std::vector<float>>> &data_mag = *scan_data_mag;
    std::vector<std::vector<std::vector<float>>> &data_t_mag = *scan_data_t_mag;
    std::vector<std::vector<float>> &temp_mag = *temp2d_mag;

    std::vector<std::vector<std::vector<float>>> &data_phase = *scan_data_phase;
    std::vector<std::vector<std::vector<float>>> &data_t_phase = *scan_data_t_phase;
    std::vector<std::vector<float>> &temp_phase = *temp2d_phase;

    double m{}, p{};
    if(y_comp || (x_comp && y_comp)){
        if(c_scan_com == YCOMP){
            for(size_t yIndex=0; yIndex<data_mag.size(); yIndex++){
                for(size_t xIndex=0; xIndex<x_points; xIndex++){
                    m=data_mag[yIndex][xIndex][fr];
                    p=data_phase[yIndex][xIndex][fr];
                    colorMap_y_mag->data()->setCell(xIndex, y_points-yIndex-1, m);
                    colorMap_y_phase->data()->setCell(xIndex, y_points-yIndex-1, p);
                }
            }
            for(size_t xIndex=0; xIndex<temp_mag.size(); xIndex++){
                colorMap_y_mag->data()->setCell(xIndex, y_points-data_mag.size()-1, temp_mag[xIndex][fr]);
                colorMap_y_phase->data()->setCell(xIndex, y_points-data_phase.size()-1, temp_phase[xIndex][fr]);
            }
        }else if(c_scan_com == XCOMP){
            for(size_t yIndex=0; yIndex<data_t_mag.size(); yIndex++){
                for(size_t xIndex=0; xIndex<x_points; xIndex++){
                    m=data_t_mag[yIndex][xIndex][fr];
                    p=data_t_phase[yIndex][xIndex][fr];
                    colorMap_y_mag->data()->setCell(xIndex, y_points-yIndex-1, m);
                    colorMap_y_phase->data()->setCell(xIndex, y_points-yIndex-1, p);
                }
            }
        }
    }

    if(x_comp || (x_comp && y_comp)){
        if(c_scan_com == XCOMP){
            for(size_t yIndex=0; yIndex<data_mag.size(); yIndex++){
                for(size_t xIndex=0; xIndex<x_points; xIndex++){
                    m=data_mag[yIndex][xIndex][fr];
                    p=data_phase[yIndex][xIndex][fr];
                    colorMap_x_mag->data()->setCell(xIndex, y_points-yIndex-1, m);
                    colorMap_x_phase->data()->setCell(xIndex, y_points-yIndex-1, p);
                }
            }
            for(size_t xIndex=0; xIndex<temp_mag.size(); xIndex++){
                colorMap_x_mag->data()->setCell(xIndex, y_points-data_mag.size()-1, temp_mag[xIndex][fr]);
                colorMap_x_phase->data()->setCell(xIndex, y_points-data_phase.size()-1, temp_phase[xIndex][fr]);
            }
        }
    }
    ui->nearfield_plot->replot();
}

void NF_plot_vna::on_interpolate_radio_clicked(bool checked)
{
    if(y_comp || (x_comp && y_comp)){
        colorMap_y_mag->setInterpolate(checked);
        colorMap_y_phase->setInterpolate(checked);
    }
    if(x_comp || (x_comp && y_comp)){
        colorMap_x_mag->setInterpolate(checked);
        colorMap_x_phase->setInterpolate(checked);
    }
    this->replot_heatmap();
}

void NF_plot_vna::on_save_selected_button_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Save plots"), "/home");
    double f{};
    if(path.length() != 0){
        if(multiple_export_time){
            for(auto &p : exp_points){
                fr = p.x();

                f = freq->at(fr)/1e6;
                f = std::ceil(f * 100.0) / 100.0;
                QString message = QString("Frequency: %1 MHz").arg(f);
                fr_label->setText(message);

                this->replot_heatmap();
                ui->nearfield_plot->rescaleAxes();
                QCPRange r_m((int)p.y()-40, (int)p.y());
                QCPRange r_p(-180.0, 180.0);
                colorScale_x_mag->setDataRange(r_m);
                colorScale_x_phase->setDataRange(r_p);
                colorScale_y_mag->setDataRange(r_m);
                colorScale_y_phase->setDataRange(r_p);
                ui->nearfield_plot->savePng(path+"/"+dut_name+"_"+QString::number(f)+"MHz.png", ui->nearfield_plot->minimumWidth(), ui->nearfield_plot->minimumHeight());
            }
        }else{
            ui->nearfield_plot->savePng(path+"/"+dut_name+"_"+QString::number(f)+"MHz.png", ui->nearfield_plot->minimumWidth(), ui->nearfield_plot->minimumHeight());
        }
        ui->maxhold_plot->savePng(path+"/"+dut_name+"_maxhold.png", 650, ui->nearfield_plot->minimumHeight());
        ui->export_plots_button->setEnabled(true);
        ui->save_selected_button->setVisible(false);
        ui->cancel_saving_button->setVisible(false);
        ui->custom_radioButton->setVisible(false);
        ui->default_radioButton->setVisible(false);
        dwPoints->data()->clear();
        ui->maxhold_plot->replot();
        exp_points.clear();
        multiple_export_time = false;
        QMessageBox::information(this, "Export plots", "Plot(s) exported!");
    }
}

void NF_plot_vna::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
    if(!m_selected)
        m_selected=true;

    double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
    double f = freq->at(dataIndex)/1e6;
    f = std::ceil(f * 100.0) / 100.0;
    fr = dataIndex;
    QString message = QString("Frequency: %1 MHz").arg(f);
    fr_label->setText(message);
    if(multiple_export_time){
        exp_points.push_back(QPoint(dataIndex, dataValue));
    }
    this->add_single_marker(fr);
    this->replot_heatmap();
}

void NF_plot_vna::freq_labelDoubleClicked(QMouseEvent *event)
{
    Q_UNUSED(event);

    bool ok1{false};
    float fr1 = freq->at(fr)/1e6;
    fr1 = std::ceil(fr1* 100.0) / 100.0;

    float sel = QInputDialog::getDouble(this, "Select frequency", "Frequency [MHz] ", fr1, 0.0, 2000.0, 2, &ok1, Qt::WindowSystemMenuHint);
    sel = sel*1e6;
    if(ok1){

        if(!m_selected)
            m_selected=true;

        int ix{};
        float diff{};
        float min=std::numeric_limits<float>::infinity();

        for(uint i=0; i<freq->size(); i++){
            diff = std::abs(sel-freq->at(i));
            if(diff < min){
                min = diff;
                ix = i;
            }
        }
        float f_freq = freq->at(ix)/1e6;
        f_freq = std::ceil(f_freq * 100.0) / 100.0;

        QString message1 = QString("%1 MHz").arg(f_freq);
        fr_label->setText("Frequency: "+message1);
        fr = ix;

        this->add_single_marker(ix);
        this->replot_heatmap();
    }
}

void NF_plot_vna::on_testSlider_valueChanged(int v)
{
    QImage temp = pcb_img_cpy;
    temp = temp.convertToFormat(QImage::Format_ARGB32);
    QPainter p;
    p.begin(&temp);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(temp.rect(), QColor(0, 0, 0, v));
    p.end();
    image_pixmap_x_mag->setPixmap(QPixmap::fromImage(temp));
    image_pixmap_y_mag->setPixmap(QPixmap::fromImage(temp));
    image_pixmap_x_phase->setPixmap(QPixmap::fromImage(temp));
    image_pixmap_y_phase->setPixmap(QPixmap::fromImage(temp));
    this->replot_heatmap();
}

void NF_plot_vna::axisLabelDoubleClick(QCPAxis *ax, QCPAxis::SelectablePart part)
{
    Q_UNUSED(part);
    double upper{}, lower{};

    if(ax == colorScale_x_mag->axis()){
        bool ok1{false}, ok2{false};

        upper = QInputDialog::getDouble(this, "X magnitude dynamic range", "Input upper limit: ", colorScale_x_mag->dataRange().upper, -200.0, 200.0, 1, &ok1, Qt::WindowSystemMenuHint);
        if(ok1)
            lower = QInputDialog::getDouble(this, "X magnitude dynamic range", "Input lower limit: ", colorScale_x_mag->dataRange().lower, -200.0, 200.0, 1, &ok2, Qt::WindowSystemMenuHint);

        if(ok1&&ok2){
          QCPRange range(lower, upper);
          colorMap_x_mag->setDataRange(range);
          ui->nearfield_plot->replot();
        }
    }else if(ax == colorScale_y_mag->axis()){
        bool ok1{false}, ok2{false};

        upper = QInputDialog::getDouble(this, "Y magnitude dynamic range", "Input upper limit: ", colorScale_y_mag->dataRange().upper, -200.0, 200.0, 1, &ok1, Qt::WindowSystemMenuHint);
        if(ok1)
            lower = QInputDialog::getDouble(this, "Y magnitude dynamic range", "Input lower limit: ", colorScale_y_mag->dataRange().lower, -200.0, 200.0, 1, &ok2, Qt::WindowSystemMenuHint);

        if(ok1&&ok2){
          QCPRange range(lower, upper);
          colorMap_y_mag->setDataRange(range);
          ui->nearfield_plot->replot();
        }
    }else if(ax == colorScale_x_phase->axis()){
        bool ok1{false}, ok2{false};

        upper = QInputDialog::getDouble(this, "X phase dynamic range", "Input upper limit: ", colorScale_x_phase->dataRange().upper, -180.0, 180.0, 1, &ok1, Qt::WindowSystemMenuHint);
        if(ok1)
            lower = QInputDialog::getDouble(this, "X phase dynamic range", "Input lower limit: ", colorScale_x_phase->dataRange().lower, -180.0, 180.0, 1, &ok2, Qt::WindowSystemMenuHint);

        if(ok1&&ok2){
          QCPRange range(lower, upper);
          colorMap_x_phase->setDataRange(range);
          ui->nearfield_plot->replot();
        }
    }else if(ax == colorScale_y_phase->axis()){
        bool ok1{false}, ok2{false};

        upper = QInputDialog::getDouble(this, "Y phase dynamic range", "Input upper limit: ", colorScale_y_phase->dataRange().upper, -180.0, 180.0, 1, &ok1, Qt::WindowSystemMenuHint);
        if(ok1)
            lower = QInputDialog::getDouble(this, "Y phase dynamic range", "Input lower limit: ", colorScale_y_phase->dataRange().lower, -180.0, 180.0, 1, &ok2, Qt::WindowSystemMenuHint);

        if(ok1&&ok2){
          QCPRange range(lower, upper);
          colorMap_y_phase->setDataRange(range);
          ui->nearfield_plot->replot();
        }
    }
}


void NF_plot_vna::static_plot_init(QString& path, bool x, bool y)
{
    x_comp = x; y_comp = y;

    std::ifstream *file = new std::ifstream;
    if(y_comp){
        file->open((path+"/y_comp_magn_scan_data_tensor.bin").toLocal8Bit(), std::ios::binary);
    }else{
        file->open((path+"/x_comp_magn_scan_data_tensor.bin").toLocal8Bit(), std::ios::binary);
    }
    if(file->is_open()){
        file->seekg (0, file->end);
        int length = file->tellg();
        file->seekg (0, file->beg);

        char * buf = new char[length];
        file->read(buf, length);

        const float* ptrFloat = reinterpret_cast<const float*>(buf);
        step_size = ptrFloat[0];
        static_sp = ptrFloat[6];
        x_points = ptrFloat[7];
        y_points = ptrFloat[8];
        delete[] buf;
    }

    this->init_plot(x_points, y_points);
    this->maxhold_init(static_sp);
    this->static_data_read(path);
    this->static_maxhold_plot();
    this->display_first_freq(freq->at(0), freq->at(freq->size()-1));
    this->replot_heatmap();
    this->show();
}

void NF_plot_vna::static_data_read(QString& path)
{
    std::ifstream *file = new std::ifstream;
    std::ifstream *file2 = new std::ifstream;

    file2->open((path + "/xaxis_data.bin").toLocal8Bit(), std::ios::binary);

    //Read frequencies data
    if(file2->is_open()){
        file2->seekg (0, file2->end);
        int length = file2->tellg();
        file2->seekg (0, file2->beg);

        char * buf2 = new char[length];
        file2->read(buf2, length);

        const float* ptrFloat_2 = reinterpret_cast<const float*>(buf2);

        for(uint i=0; i<static_sp; i++){
            st_freq.push_back(ptrFloat_2[i]);
        }
        this->display_first_freq(st_freq.at(0), st_freq.at(st_freq.size()-1));
        delete[] buf2;
    }
    delete file2;

    //Y component data
    if(y_comp){

        //Magnitude data
        c_scan_com = YCOMP;
        file->open((path + "/y_comp_magn_scan_data_tensor.bin").toLocal8Bit(), std::ios::binary);

        if(file->is_open()){

            file->seekg (0, file->end);
            int length = file->tellg();
            file->seekg (0, file->beg);

            char * buf = new char[length];
            file->read(buf, length);

            const float* ptrFloat = reinterpret_cast<const float*>(buf);

            uint x_v = 0;
            uint y_v = 0;

            for(uint i=0; i<(x_points*y_points*static_sp); i++){
                st_mag.push_back(ptrFloat[9+i]);
                if(((i+1)%(uint)static_sp) == 0){
                    st_temp2d_mag.push_back(st_mag);

                    for(size_t y=0; y<(size_t)st_freq.size(); y++){
                        if(st_mag.at(y) > maxh[y])
                            maxh[y] = st_mag.at(y);
                    }
                    st_mag.clear();
                    x_v++;
                    if(x_v == x_points){
                        st_scan_data_mag.push_back(st_temp2d_mag);
                        st_temp2d_mag.clear();
                        x_v=0;
                        y_v++;
                    }
                }
            }
            delete[] buf;
            file->close();
        }

        st_scan_data_t_mag = st_scan_data_mag;
        st_temp2d_mag.clear();
        st_mag.clear();

        delete file;
        file = nullptr;

        //Phase data
        file = new std::ifstream;
        file->open((path + "/y_comp_phase_scan_data_tensor.bin").toLocal8Bit(), std::ios::binary);

        if(file->is_open()){

            file->seekg (0, file->end);
            int length = file->tellg();
            file->seekg (0, file->beg);

            char * buf = new char[length];
            file->read(buf, length);

            const float* ptrFloat = reinterpret_cast<const float*>(buf);

            uint x_v = 0;
            uint y_v = 0;

            for(uint i=0; i<(x_points*y_points*static_sp); i++){
                st_phase.push_back(ptrFloat[9+i]);
                if(((i+1)%(uint)static_sp) == 0){
                    st_temp2d_phase.push_back(st_phase);
                    st_phase.clear();
                    x_v++;
                    if(x_v == x_points){
                        st_scan_data_phase.push_back(st_temp2d_phase);
                        st_temp2d_phase.clear();
                        x_v=0;
                        y_v++;
                    }
                }
            }
            delete[] buf;
            file->close();
        }
        st_scan_data_t_phase = st_scan_data_phase;

        st_temp2d_phase.clear();
        st_phase.clear();

        if(x_comp && y_comp){
            st_scan_data_mag.clear();
            st_scan_data_phase.clear();
        }
    }

    if(x_comp){

        c_scan_com = XCOMP;
        file = new std::ifstream;
        file->open((path + "/x_comp_magn_scan_data_tensor.bin").toLocal8Bit(), std::ios::binary);

        if(file->is_open()){

            file->seekg (0, file->end);
            int length = file->tellg();
            file->seekg (0, file->beg);

            char * buf = new char[length];
            file->read(buf, length);

            const float* ptrFloat = reinterpret_cast<const float*>(buf);

            uint x_v = 0;
            uint y_v = 0;

            for(uint i=0; i<(x_points*y_points*static_sp); i++){
                st_mag.push_back(ptrFloat[9+i]);
                if(((i+1)%static_sp) == 0){
                    st_temp2d_mag.push_back(st_mag);

                    for(size_t y=0; y<(size_t)st_freq.size(); y++){
                        if(st_mag.at(y) > maxh[y])
                            maxh[y] = st_mag.at(y);
                    }
                    st_mag.clear();
                    x_v++;
                    if(x_v == x_points){
                        st_scan_data_mag.push_back(st_temp2d_mag);
                        st_temp2d_mag.clear();
                        x_v=0;
                        y_v++;
                    }
                }
            }
            delete[] buf;
            file->close();
            delete file;
            file = nullptr;


            //Phase data
            file = new std::ifstream;
            file->open((path + "/x_comp_phase_scan_data_tensor.bin").toLocal8Bit(), std::ios::binary);

            if(file->is_open()){

                file->seekg (0, file->end);
                int length = file->tellg();
                file->seekg (0, file->beg);

                char * buf = new char[length];
                file->read(buf, length);

                const float* ptrFloat = reinterpret_cast<const float*>(buf);

                uint x_v = 0;
                uint y_v = 0;

                for(uint i=0; i<(x_points*y_points*static_sp); i++){
                    st_phase.push_back(ptrFloat[9+i]);
                    if(((i+1)%(uint)static_sp) == 0){
                        st_temp2d_phase.push_back(st_phase);
                        st_phase.clear();
                        x_v++;
                        if(x_v == x_points){
                            st_scan_data_phase.push_back(st_temp2d_phase);
                            st_temp2d_phase.clear();
                            x_v=0;
                            y_v++;
                        }
                    }
                }
                delete[] buf;
                file->close();
            }

        }
    }
}
