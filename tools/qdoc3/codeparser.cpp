/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*
  codeparser.cpp
*/

#include "codeparser.h"
#include "node.h"
#include "tree.h"
#include "config.h"
#include "generator.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

#define COMMAND_COMPAT                  Doc::alias(QLatin1String("compat"))
#define COMMAND_DEPRECATED              Doc::alias(QLatin1String("deprecated")) // ### don't document
#define COMMAND_INGROUP                 Doc::alias(QLatin1String("ingroup"))
#define COMMAND_INMODULE                Doc::alias(QLatin1String("inmodule"))  // ### don't document
#define COMMAND_INQMLMODULE             Doc::alias(QLatin1String("inqmlmodule"))
#define COMMAND_INTERNAL                Doc::alias(QLatin1String("internal"))
#define COMMAND_MAINCLASS               Doc::alias(QLatin1String("mainclass"))
#define COMMAND_NONREENTRANT            Doc::alias(QLatin1String("nonreentrant"))
#define COMMAND_OBSOLETE                Doc::alias(QLatin1String("obsolete"))
#define COMMAND_PAGEKEYWORDS            Doc::alias(QLatin1String("pagekeywords"))
#define COMMAND_PRELIMINARY             Doc::alias(QLatin1String("preliminary"))
#define COMMAND_INPUBLICGROUP           Doc::alias(QLatin1String("inpublicgroup"))
#define COMMAND_REENTRANT               Doc::alias(QLatin1String("reentrant"))
#define COMMAND_SINCE                   Doc::alias(QLatin1String("since"))
#define COMMAND_SUBTITLE                Doc::alias(QLatin1String("subtitle"))
#define COMMAND_THREADSAFE              Doc::alias(QLatin1String("threadsafe"))
#define COMMAND_TITLE                   Doc::alias(QLatin1String("title"))

QString CodeParser::currentSubDir_;
QList<CodeParser *> CodeParser::parsers;
bool CodeParser::showInternal = false;
QMap<QString,QString> CodeParser::nameToTitle;

/*!
  The constructor adds this code parser to the static
  list of code parsers.
 */
CodeParser::CodeParser()
{
    parsers.prepend(this);
}

/*!
  The destructor removes this code parser from the static
  list of code parsers.
 */
CodeParser::~CodeParser()
{
    parsers.removeAll(this);
}

/*!
  Initialize the code parser base class.
 */
void CodeParser::initializeParser(const Config& config)
{
    showInternal = config.getBool(QLatin1String(CONFIG_SHOWINTERNAL));
}

/*!
  Terminating a code parser is trivial.
 */
void CodeParser::terminateParser()
{
    // nothing.
}

QStringList CodeParser::headerFileNameFilter()
{
    return sourceFileNameFilter();
}

void CodeParser::parseHeaderFile(const Location& location,
                                 const QString& filePath,
                                 Tree *tree)
{
    parseSourceFile(location, filePath, tree);
}

void CodeParser::doneParsingHeaderFiles(Tree *tree)
{
    doneParsingSourceFiles(tree);
}

/*!
  All the code parsers in the static list are initialized here,
  after the qdoc configuration variables have been set.
 */
void CodeParser::initialize(const Config& config)
{
    QList<CodeParser *>::ConstIterator p = parsers.begin();
    while (p != parsers.end()) {
        (*p)->initializeParser(config);
        ++p;
    }
}

/*!
  All the code parsers in the static list are terminated here.
 */
void CodeParser::terminate()
{
    QList<CodeParser *>::ConstIterator p = parsers.begin();
    while (p != parsers.end()) {
        (*p)->terminateParser();
        ++p;
    }
}

CodeParser *CodeParser::parserForLanguage(const QString& language)
{
    QList<CodeParser *>::ConstIterator p = parsers.begin();
    while (p != parsers.end()) {
        if ((*p)->language() == language)
            return *p;
        ++p;
    }
    return 0;
}

CodeParser *CodeParser::parserForHeaderFile(const QString &filePath)
{
    QString fileName = QFileInfo(filePath).fileName();

    QList<CodeParser *>::ConstIterator p = parsers.begin();
    while (p != parsers.end()) {

        QStringList headerPatterns = (*p)->headerFileNameFilter();
        foreach (QString pattern, headerPatterns) {
            QRegExp re(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
            if (re.exactMatch(fileName))
                return *p;
        }
        ++p;
    }
    return 0;
}

CodeParser *CodeParser::parserForSourceFile(const QString &filePath)
{
    QString fileName = QFileInfo(filePath).fileName();

    QList<CodeParser *>::ConstIterator p = parsers.begin();
    while (p != parsers.end()) {

        QStringList sourcePatterns = (*p)->sourceFileNameFilter();
        foreach (QString pattern, sourcePatterns) {
            QRegExp re(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
            if (re.exactMatch(fileName))
                return *p;
        }
        ++p;
    }
    return 0;
}

/*!
  Returns the set of strings representing the common metacommands.
 */
QSet<QString> CodeParser::commonMetaCommands()
{
    return QSet<QString>() << COMMAND_COMPAT
                           << COMMAND_DEPRECATED
                           << COMMAND_INGROUP
                           << COMMAND_INMODULE
                           << COMMAND_INQMLMODULE
                           << COMMAND_INTERNAL
                           << COMMAND_MAINCLASS
                           << COMMAND_NONREENTRANT
                           << COMMAND_OBSOLETE
                           << COMMAND_PAGEKEYWORDS
                           << COMMAND_PRELIMINARY
                           << COMMAND_INPUBLICGROUP
                           << COMMAND_REENTRANT
                           << COMMAND_SINCE
                           << COMMAND_SUBTITLE
                           << COMMAND_THREADSAFE
                           << COMMAND_TITLE;
}

/*!
  The topic command has been processed. Now process the other
  metacommands that were found. These are not the text markup
  commands. 
 */
void CodeParser::processCommonMetaCommand(const Location& location,
                                          const QString& command,
                                          const QString& arg,
                                          Node* node,
                                          Tree* tree)
{
    if (command == COMMAND_COMPAT) {
        location.warning(tr("\\compat command used, but Qt3 compatibility is no longer supported"));
        node->setStatus(Node::Compat);
    }
    else if (command == COMMAND_DEPRECATED) {
        node->setStatus(Node::Deprecated);
    }
    else if (command == COMMAND_INGROUP) {
        tree->addToGroup(node, arg);
    }
    else if (command == COMMAND_INPUBLICGROUP) {
        tree->addToPublicGroup(node, arg);
    }
    else if (command == COMMAND_INMODULE) {
        node->setModuleName(arg);
    }
    else if (command == COMMAND_INQMLMODULE) {
        node->setQmlModuleName(arg);
        tree->addToQmlModule(node,arg);
        QString qmid = node->qmlModuleIdentifier();
        QmlClassNode* qcn = static_cast<QmlClassNode*>(node);
        QmlClassNode::moduleMap.insert(qmid + "::" + node->name(), qcn);
    }
    else if (command == COMMAND_MAINCLASS) {
        node->setStatus(Node::Main);
    }
    else if (command == COMMAND_OBSOLETE) {
        if (node->status() != Node::Compat)
            node->setStatus(Node::Obsolete);
    }
    else if (command == COMMAND_NONREENTRANT) {
        node->setThreadSafeness(Node::NonReentrant);
    }
    else if (command == COMMAND_PRELIMINARY) {
        node->setStatus(Node::Preliminary);
    }
    else if (command == COMMAND_INTERNAL) {
        if (!showInternal) {
            node->setAccess(Node::Private);
            node->setStatus(Node::Internal);
        }
    }
    else if (command == COMMAND_REENTRANT) {
        node->setThreadSafeness(Node::Reentrant);
    }
    else if (command == COMMAND_SINCE) {
        node->setSince(arg);
    }
    else if (command == COMMAND_PAGEKEYWORDS) {
        node->addPageKeywords(arg);
    }
    else if (command == COMMAND_SUBTITLE) {
        if (node->type() == Node::Fake) {
            FakeNode *fake = static_cast<FakeNode *>(node);
            fake->setSubTitle(arg);
        }
        else
            location.warning(tr("Ignored '\\%1'").arg(COMMAND_SUBTITLE));
    }
    else if (command == COMMAND_THREADSAFE) {
        node->setThreadSafeness(Node::ThreadSafe);
    }
    else if (command == COMMAND_TITLE) {
        if (node->type() == Node::Fake) {
            FakeNode *fake = static_cast<FakeNode *>(node);
            fake->setTitle(arg);
            if (fake->subType() == Node::Example) {
                ExampleNode::exampleNodeMap.insert(fake->title(),static_cast<ExampleNode*>(fake));
            }
            nameToTitle.insert(fake->name(),arg);
        }
        else
            location.warning(tr("Ignored '\\%1'").arg(COMMAND_TITLE));
    }
}

/*!
  Find the page title given the page \a name and return it.
 */
const QString CodeParser::titleFromName(const QString& name)
{
    const QString t = nameToTitle.value(name);
    return t;
}

/*!
  \internal
 */
void CodeParser::extractPageLinkAndDesc(const QString& arg,
                                        QString* link,
                                        QString* desc)
{
    QRegExp bracedRegExp("\\{([^{}]*)\\}(?:\\{([^{}]*)\\})?");

    if (bracedRegExp.exactMatch(arg)) {
        *link = bracedRegExp.cap(1);
        *desc = bracedRegExp.cap(2);
        if (desc->isEmpty())
            *desc = *link;
    }
    else {
        int spaceAt = arg.indexOf(QLatin1Char(' '));
        if (arg.contains(".html") && spaceAt != -1) {
            *link = arg.left(spaceAt).trimmed();
            *desc = arg.mid(spaceAt).trimmed();
        }
        else {
            *link = arg;
            *desc = arg;
        }
    }
}

/*!
  \internal
 */
void CodeParser::setLink(Node* node, Node::LinkType linkType, const QString& arg)
{
    QString link;
    QString desc;
    extractPageLinkAndDesc(arg, &link, &desc);
    node->setLink(linkType, link, desc);
}

/*!
  If the \e {basedir} variable is not set in the qdocconf
  file, do nothing.

  Otherwise, search for the basedir string string in the
  \a filePath. It must be found, or else a warning message
  is output. Extract the subdirectory name that follows the
  basedir name and create a subdirectory using that name
  in the output director.
 */
void CodeParser::createOutputSubdirectory(const Location& location,
                                          const QString& filePath)
{
    QString bd = Generator::baseDir();
    if (!bd.isEmpty()) {
        int baseIdx = filePath.indexOf(bd);
        if (baseIdx == -1)
            location.warning(tr("File path: '%1' does not contain bundle base dir: '%2'")
                             .arg(filePath).arg(bd));
        else {
            int subDirIdx = filePath.indexOf(QChar('/'),baseIdx);
            if (subDirIdx == -1)
                location.warning(tr("File path: '%1' has no sub dir after bundle base dir: '%2'")
                                 .arg(filePath).arg(bd));
            else {
                ++subDirIdx;
                int fileNameIdx = filePath.indexOf(QChar('/'),subDirIdx);
                if (fileNameIdx == -1)
                    location.warning(tr("File path: '%1' has no file name after sub dir: '%2/'")
                                     .arg(filePath).arg(filePath.mid(subDirIdx)));
                else {
                    currentSubDir_ = filePath.mid(subDirIdx,fileNameIdx-subDirIdx);
                    if (currentSubDir_.isEmpty())
                        location.warning(tr("File path: '%1' has no sub dir after bundle base dir: '%2'")
                                         .arg(filePath).arg(bd));
                    else {
                        QString subDirPath = Generator::outputDir() + QLatin1Char('/') + currentSubDir_;
                        QDir dirInfo;
                        if (!dirInfo.exists(subDirPath)) {
                            if (!dirInfo.mkpath(subDirPath))
                                location.fatal(tr("Cannot create output sub-directory '%1'").arg(currentSubDir_));
                        }
                    }
                }
            }
        }
    }
}

QT_END_NAMESPACE
