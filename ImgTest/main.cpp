#include "UIImgTest.h"
#include <QtWidgets\QApplication>
#include <QtCore/QTextCodec>


int main( int argc, char *argv[] )
{
    QApplication a( argc, argv );
    
    QTextCodec *codec = QTextCodec::codecForName( "utf-8" );
    if( codec == NULL )
        codec = QTextCodec::codecForLocale();
        
    QTextCodec::setCodecForLocale( codec );
//    QTextCodec::setCodecForTr( codec );
//    QTextCodec::setCodecForCStrings( codec );
    
    UIImgTest w;
    w.show();
    return a.exec();
}
