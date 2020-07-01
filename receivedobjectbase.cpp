#include "receivedobjectbase.h"

ReceivedObjectBase::ReceivedObjectBase(QObject *parent) : QObject(parent) {
    m_IsPioneer = true;
}

ReceivedObjectBase::~ReceivedObjectBase() { }

bool ReceivedObjectBase::parseString(const QString& str, const bool is_pioneer) {
    m_IsPioneer = is_pioneer;
    return parseString(str);
}

QString ReceivedObjectBase::DecodeHexString(const QString& hex) {
    QString str = "";
    for (int i = 0; i < (int)hex.length(); i+=2) {
        int c = hex.mid(i, 2).toInt(NULL, 16);
        switch(c) {
        case 5:
            str += "[)";
            break;
        case 6:
            str += "(]";
            break;
        case 7:
            str += "I";
            break;
        case 8:
            str += "][";
            break;
        case 9:
            str += "<|";
            break;
        case 10:
            str += "|>";
            break;
        default:
            str += (QChar)c;
            break;
        }
    }
    return str;
}

