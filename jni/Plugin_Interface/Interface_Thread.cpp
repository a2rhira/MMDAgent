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

#include <locale.h>
#include <android/log.h>

#include "MMDAgent.h"

#include "Interface.h"
#include "Interface_Thread.h"

/* mainThread: main thread */
static void mainThread(void *param)
{
   Interface_Thread *open_jtalk_thread = (Interface_Thread *) param;
   open_jtalk_thread->run();
}

/* Interface_Thread::initialize: initialize thread */
void Interface_Thread::initialize()
{
   m_mmdagent = NULL;

   m_mutex = NULL;
   m_cond = NULL;
   m_thread = -1;

   m_count = 0;

   m_executing = false;
   m_kill = false;

   m_param1Buff = NULL;
   m_param2Buff = NULL;
   m_param3Buff = NULL;
}

/* Interface_Thread::clear: free thread */
void Interface_Thread::clear()
{
   int i;

   stop();
   m_kill = true;

   /* wait */
   if (m_cond != NULL)
      glfwSignalCond(m_cond);

   if(m_mutex != NULL || m_cond != NULL || m_thread >= 0) {
      if(m_thread >= 0) {
         glfwWaitThread(m_thread, GLFW_WAIT);
         glfwDestroyThread(m_thread);
      }
      if(m_cond != NULL)
         glfwDestroyCond(m_cond);
      if(m_mutex != NULL)
         glfwDestroyMutex(m_mutex);
      glfwTerminate();
   }

   if(m_param1Buff) free(m_param1Buff);
   if(m_param2Buff) free(m_param2Buff);
   if(m_param3Buff) free(m_param3Buff);

   initialize();
}

/* Interface_Thread::Interface_Thread: thread constructor */
Interface_Thread::Interface_Thread()
{
   initialize();
}

/* Interface_Thread::~Interface_Thread: thread destructor */
Interface_Thread::~Interface_Thread()
{
   clear();
}

/* Interface_Thread::loadAndStart: load models and start thread */
bool Interface_Thread::loadAndStart(MMDAgent *mmdagent, const char *dicDir, const char *config)
{
   /* load */
   if (m_interface.load(dicDir, config) != true) {
      clear();
      return false;
   }
 
   m_mmdagent = mmdagent;

   /* start thread */
   glfwInit();
   m_mutex = glfwCreateMutex();
   m_cond = glfwCreateCond();
   m_thread = glfwCreateThread(mainThread, this);
   if(m_mutex == NULL || m_cond == NULL || m_thread < 0) {
      clear();
      return false;
   }

   return true;
}

/* Interface_Thread::stopAndRelease: stop thread and free Open JTalk */
void Interface_Thread::stopAndRelease()
{
   clear();
}

/* Interface_Thread::run: main thread loop for TTS */
void Interface_Thread::run()
{
   char lip[MMDAGENT_MAXBUFLEN];
   char *param1, *param2, *param3;
   char *fileName;
   double timeout = GLFW_INFINITY;

   while (m_kill == false) {
		if (timeout == GLFW_INFINITY ){
			/* wait */
			glfwLockMutex(m_mutex);
			while(m_count <= 0) {
			 glfwWaitCond(m_cond, m_mutex, GLFW_INFINITY);
			 if(m_kill == true)
				return;
			}
			param1 = MMDAgent_strdup(m_param1Buff);
			param2 = MMDAgent_strdup(m_param2Buff);
			param3 = MMDAgent_strdup(m_param3Buff);
			m_count--;
			glfwUnlockMutex(m_mutex);

			m_executing = true;
			
			if(MMDAgent_strequal(param1, INTERFACETHREAD_TYPE_LOOP)) {
				timeout = MMDAgent_str2double(param2);
				m_interface.loopInit(param3);
				m_mmdagent->sendMessage(INTERFACETHREAD_COMMANDLOOPSTART, "%s|%s|%s", param1, param2, param3);
			} else {

				/* execute */
				__android_log_print(ANDROID_LOG_INFO, "native-activity", "m_interface.execute() :\n");
				m_interface.execute(param2, param3);
				m_mmdagent->sendMessage(INTERFACETHREAD_COMMANDEXE, "%s|%s|%s", param1, param2, param3);
			}
			if(param1) free(param1);
			if(param2) free(param2);
			if(param3) free(param3);
			
			m_executing = false;
		} else {
			//loop
//			glfwLockMutex(m_mutex);
			glfwWaitCond(m_cond, m_mutex, timeout);
			if(m_kill == true)
				return;

//			glfwUnlockMutex(m_mutex);
			m_interface.loopMain();
			fileName = m_interface.getUpdateFile();
			if (fileName != NULL){
				m_mmdagent->sendMessage(MMDAGENT_COMMAND_FST_UPDATE, "%s", fileName);
			   __android_log_print(ANDROID_LOG_INFO, "native-activity", "UpdateFileX :%s\n" , fileName);
			}
		}
   }
}

/* Interface_Thread::isRunning: check running */
bool Interface_Thread::isRunning()
{
   if(m_kill == true || m_mutex == NULL || m_cond == NULL || m_thread < 0)
      return false;
   else
      return true;
}

/* Interface_Thread::isExecuting: */
bool Interface_Thread::isExecuting()
{
   return m_executing;
}

/* checkParameter: check Parameter */
bool Interface_Thread::checkParameter(const char *param1)
{
   bool ret;

   /* check */
   if(isRunning() == false)
      return false;

   /* wait buffer mutex */
   glfwLockMutex(m_mutex);

   /* save param1, param2, and param3 */
   ret = MMDAgent_strequal(m_param1Buff, param1);

   /* release buffer mutex */
   glfwUnlockMutex(m_mutex);

   return ret;
}

/* Interface_Thread::start: start  */
void Interface_Thread::start(const char *param1, const char *param2, const char *param3)
{
   /* check */
   if(isRunning() == false)
      return;
   if(MMDAgent_strlen(param1) <= 0 || MMDAgent_strlen(param2) <= 0 || MMDAgent_strlen(param3) <= 0)
      return;

   /* wait buffer mutex */
   glfwLockMutex(m_mutex);

   /* save param1, param2, and param3 */
   if(m_param1Buff) free(m_param1Buff);
   if(m_param2Buff) free(m_param2Buff);
   if(m_param3Buff) free(m_param3Buff);
   m_param1Buff = MMDAgent_strdup(param1);
   m_param2Buff = MMDAgent_strdup(param2);
   m_param3Buff = MMDAgent_strdup(param3);
   m_count++;

   /* start thread */
   if(m_count <= 1)
      glfwSignalCond(m_cond);

   /* release buffer mutex */
   glfwUnlockMutex(m_mutex);
}

/* Interface_Thread::stop: barge-in function */
void Interface_Thread::stop()
{
   if(isRunning() == true) {
      m_interface.stop();

      /* wait buffer mutex */
      glfwLockMutex(m_mutex);

      /* stop lip sync */
      m_mmdagent->sendMessage(INTERFACETHREAD_COMMANDSTOP, "%s", m_param1Buff);

      /* release buffer mutex */
      glfwUnlockMutex(m_mutex);

   }
}
