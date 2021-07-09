#include "nf_plot_static.h"

NF_plot_static::NF_plot_static()
{
    static_plot = new NF_plot_sa;
    bool x_comp{}, y_comp{};

    bool error = false;
    QMessageBox::information(static_plot, "Load plot data", "Select folder with data files");
    QString path = QFileDialog::getExistingDirectory(static_plot, "Read data", "/home", QFileDialog::ReadOnly);

    delete static_plot;
    std::ifstream *file = new std::ifstream((path+"/xaxis_data.bin").toLocal8Bit());

    if(file->good()){

        delete file;
        file = nullptr;
        file = new std::ifstream((path+"/y_comp_scan_data_tensor.bin").toLocal8Bit());
        y_comp = file->good();
        delete file;
        file = nullptr;
        file = new std::ifstream((path+"/x_comp_scan_data_tensor.bin").toLocal8Bit());
        x_comp = file->good();
        delete file;
        file = nullptr;

        if(!(x_comp || y_comp)){
            static_plot = new NF_plot_vna;
            file = new std::ifstream((path+"/y_comp_magn_scan_data_tensor.bin").toLocal8Bit());
            y_comp = file->good();
            delete file;
            file = nullptr;
            file = new std::ifstream((path+"/x_comp_magn_scan_data_tensor.bin").toLocal8Bit());
            x_comp = file->good();
            delete file;
            file = nullptr;
        }else{
            static_plot = new NF_plot_sa;
        }

        if(!(x_comp || y_comp)){
            QMessageBox::critical(static_plot, "Load error", "Wrong data files selected");
            error = true;
        }

    }else{
        QMessageBox::critical(static_plot, "Load error", "Wrong data files selected");
        error = true;
    }

    if(!error){
        QImage *cropped_image = new QImage(path+"/cropped_image.png");
        if(!cropped_image->isNull()){
            static_plot->add_bg_image(cropped_image);
        }else{
            QMessageBox::critical(static_plot, "Image error", "No background image selected");
            error = true;
            delete cropped_image;
        }
        static_plot->setAttribute(Qt::WA_DeleteOnClose);
        static_plot->static_data_vectors_init();
        static_plot->static_plot_init(path, x_comp, y_comp);
    }
    delete file;
    file = nullptr;
}
