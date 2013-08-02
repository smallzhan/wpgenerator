#if 1
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
#else
#include "wpencoder.h"
using namespace WaterPrinter;

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("please input two .3dwp files\n");
        exit(0);
    }
    WPEncoder encoder;
    encoder.setOutfile(argv[1]);
    encoder.setCryptType(0);
    encoder.decodeBits();

    encoder.addFile(argv[2]);
    std::string out_str = encoder.getBitmap();

    //const char* p_bits = out_str.c_str();
    for (size_t i = 0; i < out_str.size(); ++i)
    {
        if (i % 8 == 0)
        {
            printf("\n");
        }
        char digit = out_str[i];
        for (int j = 7; j >= 0; --j)
        {
            if (digit & (1 << j))
                printf("#");
            else
                printf("-");
        }
    }
    printf("\n");
}
#endif
