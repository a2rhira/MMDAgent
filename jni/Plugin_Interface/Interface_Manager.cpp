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

#include "MMDAgent.h"
#include <android/log.h>

#include "Interface.h"
#include "Interface_Thread.h"
#include "Interface_Manager.h"

/* Interface_Event_initialize: initialize input message buffer */
static void Interface_Event_initialize(Interface_Event *e, const char *str)
{
   if (str != NULL)
      e->event = MMDAgent_strdup(str);
   else
      e->event = NULL;
   e->next = NULL;
}

/* Interface_Event_clear: free input message buffer */
static void Interface_Event_clear(Interface_Event *e)
{
   if (e->event != NULL)
      free(e->event);
   Interface_Event_initialize(e, NULL);
}

/* Interface_EventQueue_initialize: initialize queue */
static void Interface_EventQueue_initialize(Interface_EventQueue *q)
{
   q->head = NULL;
   q->tail = NULL;
}

/* Interface_EventQueue_clear: free queue */
static void Interface_EventQueue_clear(Interface_EventQueue *q)
{
   Interface_Event *tmp1, *tmp2;

   for (tmp1 = q->head; tmp1 != NULL; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      Interface_Event_clear(tmp1);
      free(tmp1);
   }
   Interface_EventQueue_initialize(q);
}

/* Interface_EventQueue_enqueue: enqueue */
static void Interface_EventQueue_enqueue(Interface_EventQueue *q, const char *str)
{
   if(MMDAgent_strlen(str) <= 0)
      return;

   if (q->tail == NULL) {
      q->tail = (Interface_Event *) calloc(1, sizeof (Interface_Event));
      Interface_Event_initialize(q->tail, str);
      q->head = q->tail;
   } else {
      q->tail->next = (Interface_Event *) calloc(1, sizeof (Interface_Event));
      Interface_Event_initialize(q->tail->next, str);
      q->tail = q->tail->next;
   }
}

/* Interface_EventQueue_dequeue: dequeue */
static void Interface_EventQueue_dequeue(Interface_EventQueue *q, char **str)
{
   Interface_Event *tmp;

   if (q->head == NULL) {
      *str = NULL;
      return;
   }
   *str = MMDAgent_strdup(q->head->event);

   tmp = q->head->next;
   Interface_Event_clear(q->head);
   free(q->head);
   q->head = tmp;
   if (tmp == NULL)
      q->tail = NULL;
}

/* mainThread: main thread */
static void mainThread(void *param)
{
   Interface_Manager *interface_manager = (Interface_Manager *) param;
   interface_manager->run();
}

/* Interface_Manager::initialize: initialize */
void Interface_Manager::initialize()
{
   m_mmdagent = NULL;

   m_mutex = NULL;
   m_cond = NULL;
   m_thread = -1;

   m_count = 0;

   m_list = NULL;

   m_dicDir = NULL;
   m_config = NULL;

   m_kill = false;

   Interface_EventQueue_initialize(&m_bufferQueue);
}

/* Interface_Manager::clear clear */
void Interface_Manager::clear()
{
   Interface_Link *tmp1, *tmp2;

   m_kill = true;

   /* stop and release all thread */
   for(tmp1 = m_list; tmp1; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      tmp1->interface_thread.stopAndRelease();
      delete tmp1;
   }

   /* wait */
   if(m_cond != NULL)
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

   if(m_dicDir)
      free(m_dicDir);
   if(m_config)
      free(m_config);

   Interface_EventQueue_clear(&m_bufferQueue);

   initialize();
}

/* Interface_Manager::Interface_Manager: constructor */
Interface_Manager::Interface_Manager()
{
   initialize();
}

/* Interface_Manager::~Interface_Manager: destructor */
Interface_Manager::~Interface_Manager()
{
   clear();
}

/* Interface_Manager::loadAndStart: load and start thread */
void Interface_Manager::loadAndStart(MMDAgent *mmdagent, const char *dicDir, const char *config)
{
   clear();

   m_mmdagent = mmdagent;
   m_dicDir = MMDAgent_strdup(dicDir);
   m_config = MMDAgent_strdup(config);

   if(m_mmdagent == NULL || m_dicDir == NULL || m_config == NULL) {
      clear();
      return;
   }

   glfwInit();
   m_mutex = glfwCreateMutex();
   m_cond = glfwCreateCond();
   m_thread = glfwCreateThread(mainThread, this);
   if(m_mutex == NULL || m_cond == NULL || m_thread < 0) {
      clear();
      return;
   }
}

/* Interface_Manager::stopAndRelease: stop and release thread */
void Interface_Manager::stopAndRelease()
{
   clear();
}

/* Interface_Manager::run: main loop */
void Interface_Manager::run()
{
   int i;
   Interface_Link *link;
   char *buff, *save;
   char *param1, *param2, *param3;
   bool ret = true;

   /* create initial threads */
   for(i = 0; i < INTERFACEMANAGER_INITIALNTHREAD; i++) {
      link = new Interface_Link;
      if(link->interface_thread.loadAndStart(m_mmdagent, m_dicDir, m_config) == false)
         ret = false;
      link->next = m_list;
      m_list = link;
   }

   if(ret == false)
      return;

   while(m_kill == false) {
      glfwLockMutex(m_mutex);
      while(m_count <= 0) {
         glfwWaitCond(m_cond, m_mutex, GLFW_INFINITY);
         if(m_kill == true)
            return;
      }
      Interface_EventQueue_dequeue(&m_bufferQueue, &buff);
      m_count--;
      glfwUnlockMutex(m_mutex);

      if(buff != NULL) {
         param1 = MMDAgent_strtok(buff, "|", &save);
         param2 = MMDAgent_strtok(NULL, "|", &save);
         param3 = MMDAgent_strtok(NULL, "|", &save);

         if(param1 != NULL && param2 != NULL && param3 != NULL) {
            /* check param */
            for(i = 0, link = m_list; link; link = link->next, i++)
               if(link->interface_thread.checkParameter(param1) == true)
                  break;
            if(link) {
               if(link->interface_thread.isExecuting() == true)
                  link->interface_thread.stop(); /* if the same character is speaking, stop immediately */
            } else {
               for(i = 0, link = m_list; link; link = link->next, i++)
                  if(link->interface_thread.isRunning() == true && link->interface_thread.isExecuting() == false)
                     break;
               if(link == NULL) {
                  link = new Interface_Link;
                  link->interface_thread.loadAndStart(m_mmdagent, m_dicDir, m_config);
                  link->next = m_list;
                  m_list = link;
               }
            }
            /* set */
            link->interface_thread.start(param1, param2, param3);
         }
         free(buff); /* free buffer */
      }
   }
}

/* Interface_Manager::isRunning: check running */
bool Interface_Manager::isRunning()
{
   if(m_kill == true || m_mutex == NULL || m_cond == NULL || m_thread < 0)
      return false;
   else
      return true;
}

/* Interface_Manager::start: start  */
void Interface_Manager::start(const char *str)
{
   /* check */
   if(isRunning() == false || MMDAgent_strlen(str) <= 0)
      return;

   /* wait buffer mutex */
   glfwLockMutex(m_mutex);

   /* enqueue character name, speaking style, and text */
   Interface_EventQueue_enqueue(&m_bufferQueue, str);
   m_count++;

   /* start event */
   if(m_count <= 1)
      glfwSignalCond(m_cond);

   /* release buffer mutex */
   glfwUnlockMutex(m_mutex);
}

/* Interface_Manager::stop: stop */
void Interface_Manager::stop(const char *str)
{
   Interface_Link *link;

   for(link = m_list; link; link = link->next) {
      if(link->interface_thread.checkParameter(str)) {
         link->interface_thread.stop();
         return;
      }
   }
}
