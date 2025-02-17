/*
 * exportcontext.h
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#ifndef GDREXPORT_EXPORTCONTEXT_H_
#define GDREXPORT_EXPORTCONTEXT_H_
#include <QDir>
#include <QSet>
#include <QString>
#include <QTextStream>
#include <QImage>
#include <projectproperties.h>
#include <dependencygraph.h>
extern "C" {
#include "lua.h"
}

enum DeviceFamily
{
  e_None,
  e_iOS,
  e_Android,
  e_WindowsDesktop,
  e_MacOSXDesktop,
  e_WinRT,
  e_GApp,
  e_Win32,
  e_Html5,
  e_Xml,
  e_LinuxDesktop,
  e_Linux,
};

struct ExportContext
{
    QString projectFileName_;
    ProjectProperties properties;
    QHash<QString, QString> args;
    QString platform;
    DeviceFamily deviceFamily;
	QString base;
	QString basews;
	QString templatename;
	QString templatenamews;
	QString appName;
	QDir outputDir;
        QDir exportDir;
    QList<QStringList> wildcards;
    QList<QList<QPair<QByteArray, QByteArray> > > replaceList;
    QList<QPair<QString, QString> > renameList;
    bool assetsOnly;
    bool player;
    //Encryption info
    bool encryptCode;
    bool encryptAssets;
    QByteArray codeKey;
    QByteArray assetsKey;
    //Assets info
    QSet<QString> jetset; //Android specific
    std::vector<QString> folderList;
    std::vector<std::pair<QString, QString> > fileQueue;
    std::vector<std::pair<QString, bool> > topologicalSort;
    std::set<QString> noEncryption;
    std::set<QString> noEncryptionExt;
    std::set<QString> noPackage;
    std::set<QString> noPackageAbs;
    QStringList allfiles;
    QStringList allfiles_abs;
    QStringList assetfiles;
    QStringList assetfiles_abs;
    QStringList luafiles;
    QStringList luafiles_abs;
    //Icons
    QImage *appicon;
    QImage *tvicon;
    QImage *splash_h_image;
    QImage *splash_v_image;
    //XML
	QMap<QString,QString> props; //global properties
	bool exportError;
    // LUA
    lua_State *L;
};



#endif /* GDREXPORT_EXPORTCONTEXT_H_ */
