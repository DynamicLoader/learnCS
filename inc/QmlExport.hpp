/**
 * @file QmlExport.hpp
 * @author Lu Yi
 * @brief Qml exports helpers and functions
 * @version 0.1
 * @date 2023-05-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __QMLEXPORT_HPP__
#define __QMLEXPORT_HPP__

#include <QObject>

#define QProperty_HelperEx(type, name, intType, intName, convG, convS)                                                 \
  private:                                                                                                             \
    Q_PROPERTY(type name READ get##name WRITE set##name NOTIFY name##Changed)                                          \
  public:                                                                                                              \
    type get##name()                                                                                                   \
    {                                                                                                                  \
        return convG(_##intName);                                                                                      \
    }                                                                                                                  \
    void set##name(const type &in)                                                                                     \
    {                                                                                                                  \
        _##intName = convS(in);                                                                                        \
        emit name##Changed();                                                                                          \
    }                                                                                                                  \
                                                                                                                       \
  Q_SIGNALS:                                                                                                           \
    void name##Changed();                                                                                              \
                                                                                                                       \
  private:                                                                                                             \
    intType _##intName;

#define QProperty_Helper(type, name) QProperty_HelperEx(type, name, type, name, , )

namespace WrapperTNAM_impl
{
class WrapperTNAM;
}

class WrapperRNAM;

/**
 * @brief A namespace that contains meta data type definations for QML environment
 * 
 */
namespace QmlExports
{

/**
 * @brief Type defination of TNAM Stat
 * 
 */
class TNAM_Stat_t : public QObject
{
    // clang-format off
    Q_OBJECT

    QProperty_Helper(uint16_t, LineNum)
    QProperty_Helper(uint16_t,PointNum)
    QProperty_Helper(uint16_t,TriNum)
        // clang-format on
        public : using QObject::QObject;

    friend class WrapperTNAM_impl::WrapperTNAM;
};

/**
 * @brief Type defination of RNAM Stat 
 * 
 */
class RNAM_Stat_t : public QObject
{
    // clang-format off
    Q_OBJECT
    QProperty_Helper(double,BPP)
    QProperty_Helper(double,CR)
    QProperty_Helper(double,PSNR)
    QProperty_Helper(int,BlockCount)

        // clang-format on
        public : using QObject::QObject;
        friend class WrapperRNAM;
};
} // namespace QmlExports

using TNAM_Stat_t = QmlExports::TNAM_Stat_t;
using RNAM_Stat_t = QmlExports::RNAM_Stat_t;

#endif