#include "theme.h"



Theme::Theme()
{

}



QColor Theme::color_default = QColor(0xF0F0F0);
QColor Theme::color_red = QColor(0xFF0000);
QColor Theme::color_crimson = QColor(0xC2004E);
QColor Theme::color_green = QColor(0x00FF00);
QColor Theme::color_olive = QColor(0x808000);
QColor Theme::color_blue = QColor(0x0000FF);
QColor Theme::color_lightblue = QColor(0x4169E1);
QColor Theme::color_yellow = QColor(0xFFFF00);
QColor Theme::color_khaki = QColor(0xF0E68C);
QColor Theme::color_purple = QColor(0x9400D3);
QColor Theme::color_pink = QColor(0xEE82EE);
QColor Theme::color_brown = QColor(0x8B4513);
QColor Theme::color_lightbrown = QColor(0xDEB887);

QColor Theme::color_overlay = QColor(0x606060);

QColor Theme::color_black = Qt::black;

std::vector<QColor> Theme::colors {std::initializer_list<QColor>
{
    Theme::color_red,
    Theme::color_green,
    Theme::color_blue,
    Theme::color_crimson,
    Theme::color_olive,
    Theme::color_lightblue,
    Theme::color_yellow,
    Theme::color_purple,
    Theme::color_brown,
    Theme::color_khaki,
    Theme::color_pink,
    Theme::color_lightbrown
}};

