/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2013  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAgent project team nor the names of  */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* headers */
#include <android/log.h>

#include <jni.h>
#include <android_native_app_glue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include "MMDAgent.h"


#include "Interface.h"

/* Interface::initialize: initialize system */
void Interface::initialize()
{
	m_file_update = false;
	m_file = NULL;
}

/* Interface::clear: free system */
void Interface::clear()
{
}

/* Interface::Interface: constructor */
Interface::Interface()
{
   initialize();
}

/* Interface::~Interface: destructor */
Interface::~Interface()
{
   clear();
}

/* Interface::load: load dictionary and models */
bool Interface::load(const char *dicDir, const char *config)
{
	FILE *fp;
	DIR* dir;
	dirent* dp;
	int i;
	Interface_File *Files;
	struct stat statBuff;

//	execute(NULL, NULL);

   if (dicDir == NULL || config == NULL )
	  return false;

   /* load config file */
/*
   fp = MMDAgent_fopen(config, "r");
   if (fp == NULL)
	  return false;
   fclose(fp);
*/

   m_dir  = MMDAgent_strdup(dicDir);

   if (NULL == (dir = opendir(m_dir))){
	   __android_log_print(ANDROID_LOG_INFO, "native-activity", "if opendir error %s", dicDir);
	  return false;
   }

   for(i = 0; NULL != (dp = readdir(dir)); i++){
	   __android_log_print(ANDROID_LOG_INFO, "native-activity", "if readdirx  %d:%s\n" , i , dp->d_name);
	   if ((MMDAgent_strequal(dp->d_name, ".") == false && MMDAgent_strequal(dp->d_name, "..") == false ) &&
           (MMDAgent_strtailmatch(dp->d_name, ".fst") == true || MMDAgent_strtailmatch(dp->d_name, ".FST") == true)) {

		  stat(dp->d_name, &statBuff);

		  Files = new Interface_File;
		  Files->name  = MMDAgent_strdup(dp->d_name);
		  Files->ctime = (long)statBuff.st_ctime;
		  Files->next = m_file;
		  m_file = Files;
	   }
   }
   closedir(dir);

   return true;
}

/* Interface::stop: stop speech synthesis */
void Interface::stop()
{
}

/* Interface::execute:execute */
void Interface::execute(const char *param2, const char *param3)
{
    return;
}

/* Interface::execute:execute */
void Interface::executeLog(const char *param2, const char *param3)
{
    return;
}

/* Interface::loopInit:loopInit */
void Interface::loopInit(const char *param3)
{
   double sec, now;
   return;
}

/* Interface::loopMain: loopMain */
void Interface::loopMain()
{
	FILE *fp;
	DIR* dir;
	dirent* dp;
	int i, j;
	Interface_File *Files;
	Interface_File *aFiles;
	struct stat statBuff;
	bool det;
	double ctime;
	char *fileName;

	if (m_file_update)
		return;

	if (NULL == (dir = opendir(m_dir)))
		return;

	for(i = 0; NULL != (dp = readdir(dir)); i++){
	   if ((MMDAgent_strequal(dp->d_name, ".") == false && MMDAgent_strequal(dp->d_name, "..") == false ) &&
           (MMDAgent_strtailmatch(dp->d_name, ".fst") == true || MMDAgent_strtailmatch(dp->d_name, ".FST") == true)) {
			stat(dp->d_name, &statBuff);
			det = false;
			fileName =MMDAgent_strdup(dp->d_name);
			ctime = (long)statBuff.st_ctime;
			for(j = 0, Files = m_file; Files; Files = Files->next, j++){
				if(MMDAgent_strequal(fileName, Files->name) && Files->ctime == (long)ctime ){
					det = true;
					break;
				}
			}
			if (det == false){
				aFiles = new Interface_File;
				aFiles->name  = MMDAgent_strdup(fileName);
				aFiles->ctime = ctime;
				aFiles->next = m_file;
				m_file = aFiles;
				m_file_update = true;
				break;
			}
		}
   }
   closedir(dir);
	__android_log_print(ANDROID_LOG_INFO, "native-activity", "m_interface.loopMain() :\n");
}

char *Interface::getUpdateFile()
{
	char buff[MMDAGENT_MAXBUFLEN];

	if (m_file_update){
//		fileName = MMDAgent_strdup(m_file->name);
	   __android_log_print(ANDROID_LOG_INFO, "native-activity", "UpdateFile :%s\n" , m_file->name);
		m_file_update = false;
		sprintf(buff, "%s%c%s", m_dir, MMDAGENT_DIRSEPARATOR, m_file->name);
		m_file_name =MMDAgent_strdup(buff);
		return m_file_name;
	}
//	execute(NULL, NULL);
//	glfwExeClassFunction("com/example/mmdagent/HelloJni", "toastText", "()V", NULL, NULL);

	return NULL;
}
