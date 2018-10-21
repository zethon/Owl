#pragma once
#include <QtCore>

namespace owl
{

class Moment final : public QObject
{

public:
    Moment();
    Moment(const QDateTime&);
    virtual ~Moment() = default;

    const QString toString() const;
    const QString toString(const QDateTime& now) const;

    void setDateTime(const QDateTime& dt) { _dt = dt; }
    const QDateTime datetime() const { return _dt; }

private:
    int monthsTo(const QDateTime& start, const QDateTime& end) const;

    QDateTime _dt;
};

} // namespace