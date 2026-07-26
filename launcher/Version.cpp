// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2023 flowln <flowlnlnln@gmail.com>
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *  Copyright (c) 2026 Trial97 <alexandru.tripon97@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "Version.h"

#include <QDebug>
#include <QRegularExpressionMatch>
#include <QUrl>


/// qDebug print support for the Version class
QDebug operator<<(QDebug debug, const Version& v)
{
    const QDebugStateSaver saver(debug);

    debug.nospace() << "Version{ string: " << v.toString() << ", sections: [ ";

    bool first = true;
    for (const auto& s : v.m_sections) {
        if (!first) {
            debug.nospace() << ", ";
        }
        debug.nospace() << s.value;
        first = false;
    }

    debug.nospace() << " ]" << " }";

    return debug;
}

int Version::Section::compare(const Section& other) const
{
    if (this->t == Type::Numeric && other.t == Type::Numeric) {
        auto aLen = this->value.size();
        if (aLen != other.value.size()) {
            return aLen < other.value.size() ? -1 : 1;
        }
        auto cmp = QString::compare(this->value, other.value);
        if (cmp < 0) return -1;
        if (cmp > 0) return 1;
        return 0;
    }
    if (this->t == Type::Null) {
        if (other.t == Type::PreRelease) return 1;
        return -1;
    }
    if (other.t == Type::Null) {
        if (this->t == Type::PreRelease) return -1;
        return 1;
    }
    auto minLen = qMin(this->value.size(), other.value.size());
    for (int i = 0; i < minLen; i++) {
        auto a = this->value.at(i);
        auto b = other.value.at(i);
        if (a != b) {
            return a.unicode() < b.unicode() ? -1 : 1;
        }
    }
    auto szCmp = this->value.size() - other.value.size();
    return szCmp < 0 ? -1 : (szCmp > 0 ? 1 : 0);
}

namespace {
void removeLeadingZeros(QString& s)
{
    s.remove(0, std::distance(s.begin(), std::find_if_not(s.begin(), s.end(), [](QChar c) { return c == '0'; })));
}
}  // namespace

void Version::parse()
{
    auto len = m_string.size();
    for (int i = 0; i < len;) {
        Section cur(Section::Type::Textual);
        auto c = m_string.at(i);
        if (c == '+') {
            break;  // Ignore appendices
        }
        // custom: the space is special to handle the strings like "1.20 Pre-Release 1"
        // this is needed to support Modrinth versions
        if (c == '-' || c == ' ') {
            // Add dash to component
            cur.value += c;
            i++;
            // If the next rune is non-digit, mark as pre-release (requires >= 1 non-digit after dash so the component has length > 1)
            if (i < len && !m_string.at(i).isDigit()) {
                cur.t = Section::Type::PreRelease;
            }
        } else if (c.isDigit()) {
            // Mark as numeric
            cur.t = Section::Type::Numeric;
        }
        for (; i < len; i++) {
            auto r = m_string.at(i);
            if ((r.isDigit() != (cur.t == Section::Type::Numeric))   // starts a new section
                || (r == ' ' && cur.t == Section::Type::Numeric)     // custom: numeric section then a space is a pre-release
                || (r == '-' && cur.t != Section::Type::PreRelease)  // "---" is a valid pre-release component
                || r == '+') {
                // Run completed (do not consume this rune)
                break;
            }
            // Add rune to current run
            cur.value += r;
        }
        if (!cur.value.isEmpty()) {
            if (cur.t == Section::Type::Numeric) {
                removeLeadingZeros(cur.value);
            }
            m_sections.append(cur);
        }
    }
}

int Version::compare(const Version& other) const
{
    const auto size = qMax(m_sections.size(), other.m_sections.size());
    for (int i = 0; i < size; ++i) {
        auto sec1 = (i >= m_sections.size()) ? Section() : m_sections.at(i);
        auto sec2 = (i >= other.m_sections.size()) ? Section() : other.m_sections.at(i);

        if (auto cmp = sec1.compare(sec2); cmp != 0) {
            return cmp;
        }
    }
    return 0;
}
