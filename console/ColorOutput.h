#include <QString>

// usage:
// ColorOutput colout

struct ColorOutput
{
    typedef enum
    {
        PLAIN           =   0x00,
        BOLD            =   0x01,
        BACKGROUND      =   0x02,
    } TextTrait;

    QString reset() const;
    QString black(TextTrait tr = PLAIN) const;
    QString red(TextTrait tr = PLAIN) const;
    QString green(TextTrait tr = PLAIN) const;
    QString yellow(TextTrait tr = PLAIN) const;
    QString blue(TextTrait tr = PLAIN) const;
    QString magenta(TextTrait tr = PLAIN) const;
    QString cyan(TextTrait tr = PLAIN) const;
    QString white(TextTrait tr = PLAIN) const;

private:
    QString preOutput(TextTrait);
};

//std::ostream& operator<<(std::ostream& os, const T& obj)
//{
//  // write obj to stream

//  return os;
//}
