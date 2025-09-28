/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CRYPTO_H
#define CRYPTO_H

#include <QObject>

class Crypto : public QObject
{
    Q_OBJECT
public:
    unsigned char key[16];
    unsigned char salt[16] = {0x63, 0x61, 0xb8, 0x0e, 0x9b, 0xdc, 0xa6, 0x63,
                            0x8d, 0x07, 0x20, 0xf2, 0xcc, 0x56, 0x8f, 0xb9};

    explicit Crypto(QObject *parent = nullptr);
    QByteArray decryptIV(QByteArray);
    QByteArray decrypt(QByteArray);
    QByteArray encrypt(QByteArray);
signals:
};

#endif // CRYPTO_H
