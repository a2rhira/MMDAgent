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

/* definitions */

#define INTERFACETHREAD_COMMANDLOOPSTART   "IF_CMD_LOOP_START"
#define INTERFACETHREAD_COMMANDEXE         "IF_CMD_EXE"
#define INTERFACETHREAD_COMMANDSTOP        "IF_CMD_STOP"

#define INTERFACETHREAD_TYPE_LOOP     "LOOP"
#define INTERFACETHREAD_TYPE_COMMAND  "CMD"

/* Interface_Thread: thread for Open JTalk */
class Interface_Thread
{
private:

   MMDAgent *m_mmdagent;

   GLFWmutex m_mutex;
   GLFWcond m_cond;
   GLFWthread m_thread;

   int m_count;

   bool m_executing;
   bool m_kill;

   char *m_param1Buff;
   char *m_param2Buff;
   char *m_param3Buff;

   Interface m_interface;

   /* initialize: initialize thread */
   void initialize();

   /* clear: free thread */
   void clear();

public:

   /* Interface_Thraed: thread constructor */
   Interface_Thread();

   /* ~Interface_Thread: thread destructor */
   ~Interface_Thread();

   /* loadAndStart: load models and start thread */
   bool loadAndStart(MMDAgent *mmdagent, const char *dicDir, const char *config);

   /* stopAndRelease: stop thread and free Open JTalk */
   void stopAndRelease();

   /* run: main thread loop for TTS */
   void run();

   /* isRunning: check running */
   bool isRunning();

   /* isExecuting: check speaking */
   bool isExecuting();

   /* checkParameter: check parameter */
   bool checkParameter(const char *param1);

   /* start: start  */
   void start(const char *param1, const char *param2, const char *param3);

   /* stop: stop */
   void stop();
};
