#include <iostream>
#include <thread>
#include <list>
#include <vector>

using namespace std;

/**
 * ��д����������ʾ
 *
 */
namespace s1{
    class A{
    public:
        // ���յ�����Ϣ(�������)���뵽һ������
        void inMsgRecvQueue(){
            for(int i = 0; i< 100000; ++i){
                cout << "inMsgRecvQueue()ִ�С�����һ��Ԫ��" << i << endl;
                msgRecvQueue.push_back(i);
            }
        }
        // �����ݴ���Ϣ������ȡ�����߳�
        void outMsgRecvQueue(){
            for(int i = 0; i< 100000; ++i){
                if(!msgRecvQueue.empty()){
                    // ��Ϣ���в�Ϊ��
                    int command = msgRecvQueue.front();// ���ص�һ��Ԫ�أ������Ԫ���Ƿ����
                    msgRecvQueue.pop_front();
                }
                else cout << "outMsgRecvQueue()ִ�С���Ϣ����Ϊ��" << i << endl;
            }
            cout << "end " << endl;
        }
    private:
        std::list<int> msgRecvQueue;
    };
}

#include <iostream>
#include <thread>
#include <list>
#include <mutex>

using namespace std;


#define DIELOCK

/**
 * ����������÷���������ʾ��������
 *
 */
namespace s2{
    class A{
    public:
        // ���յ�����Ϣ(�������)���뵽һ������
        void inMsgRecvQueue(){
            for(int i = 0; i< 100000; ++i){
                cout << "inMsgRecvQueue()ִ�С�����һ��Ԫ��" << i << endl;
                /**
                 * ��������Ҫ����������
                 */
#ifdef DIELOCK
                std::lock(my_mutex1,my_mutex2); // �����������
                std::lock_guard<std::mutex> sbGuard1(my_mutex1,std::adopt_lock);
                std::lock_guard<std::mutex> sbGuard2(my_mutex2,std::adopt_lock);
//                my_mutex1.lock();
//                my_mutex2.lock();

                msgRecvQueue.push_back(i);
//                my_mutex2.unlock();
//                my_mutex1.unlock();

#endif

#ifndef DIELOCK
                //                my_mutex.lock();
                {
                    // scope
                    lock_guard<std::mutex> sbGuard(my_mutex);
                    msgRecvQueue.push_back(i);
                }
                my_mutex.unlock();
#endif
            }
        }

        bool outMsgLULProc(int& command){
            
#ifdef DIELOCK
//            my_mutex2.lock();
//            my_mutex1.lock();
            std::lock(my_mutex1,my_mutex2); // �����������
            // ����std::adopt_lock ��ʾ������lock��
            std::lock_guard<std::mutex> sbGuard1(my_mutex1,std::adopt_lock);
            std::lock_guard<std::mutex> sbGuard2(my_mutex2,std::adopt_lock);

            if(!msgRecvQueue.empty()){
                // ��Ϣ���в�Ϊ��
                int command = msgRecvQueue.front();// ���ص�һ��Ԫ�أ������Ԫ���Ƿ����
                msgRecvQueue.pop_front();
//                my_mutex1.unlock();
//                my_mutex2.unlock();
                return true;
            }
//            my_mutex1.unlock();
//            my_mutex2.unlock();
            return false;
#endif

#ifndef DIELOCK
//            my_mutex.lock();
//            std::lock_guard<std::mutex> sbGuard(my_mutex); //����lock_guard �Ͳ�����unlock��lock��
            if(!msgRecvQueue.empty()){
                // ��Ϣ���в�Ϊ��
                int command = msgRecvQueue.front();// ���ص�һ��Ԫ�أ������Ԫ���Ƿ����
                msgRecvQueue.pop_front();
//                my_mutex.unlock();  // �����֧ҲҪ��unlock(),�����п��ܻ�һֱ��ס
                return true;
            }
//            my_mutex.unlock();
            return false;
#endif
        }
        // �����ݴ���Ϣ������ȡ�����߳�
        void outMsgRecvQueue(){
            int command = 0;
            for(int i = 0; i< 100000; ++i){
                bool result = outMsgLULProc(command);
                if(result){
                    cout << "outMsgRecvQueue()ִ�С�ȡ��һ��Ԫ��" << command << endl;
                }
                else cout << "outMsgRecvQueue()ִ�С���Ϣ����Ϊ��" << i << endl;
            }
            cout << "end " << endl;
        }
    private:
        std::list<int> msgRecvQueue;
        std::mutex my_mutex;
        std::mutex my_mutex1;
        std::mutex my_mutex2;
    };
}

/**
 * unique_lock ���
 */
namespace s3{
    class A{
    public:
        std::unique_lock<std::mutex> rtn_unique_lock(){
            std::unique_lock<std::mutex> tmpGuard(my_mutex);
            return tmpGuard; // ������ʱ���󣬵���unique_lock �ƶ����캯��
        }
        // ���յ�����Ϣ(�������)���뵽һ������
        void inMsgRecvQueue(){
            for(int i = 0; i< 100000; ++i){
                cout << "inMsgRecvQueue()ִ�С�����һ��Ԫ��" << i << endl;
                /**
                 * ��������Ҫ����������
                 */
//                lock_guard<std::mutex> sbGuard(my_mutex);
                // unique_lock ������ȫȡ�� lock_guard
//                my_mutex.lock();
//                unique_lock<std::mutex> sbGuard1(my_mutex);
                // �ƶ����� ���ʱ���ֱ�Ӱ�����Ȩת�Ƶ���sbGuard2��
//                unique_lock<std::mutex> sbGuard2(std::move(sbGuard1));
                // Ҳ����ͨ������
                unique_lock<std::mutex> sbGuard2 = rtn_unique_lock();
                // �����ϵ
//                std::mutex *ptx = sbGuard1.release();
                std::mutex *ptx = sbGuard2.release();
                msgRecvQueue.push_back(i);
                // ��������ֶ����
                ptx->unlock();
                // �Լ������ν���
//                unique_lock<std::mutex> sbGuard1(my_mutex,std::adopt_lock);
//                unique_lock<std::mutex> sbGuard1(my_mutex,std::try_to_lock); //ʹ������ڶ����� ������֮ǰlock ����Ῠס
                // ����õ����Ļ�
//                if(sbGuard1.owns_lock()){
//                    cout << "inMsgRecvQueue ִ�У��õ���ͷ..." << endl;
//                    msgRecvQueue.push_back(i);
//                }
//                else{
//                    cout << "inMsgRecvQueue ִ�У�����û���õ���ͷ..." << endl;
//                }
//                unique_lock<std::mutex> sbGuard1(my_mutex,std::defer_lock); // ��ʼ��һ����û�м�����mymutex
//                sbGuard1.lock(); // �Լ�����
//                // ��һЩ�ǹ������Ҫ���� ������unlock()
//                sbGuard1.unlock();
//                sbGuard1.lock(); // �Լ�����
//                if(sbGuard1.try_lock()){
//                    cout << "inMsgRecvQueue ִ�У��õ���ͷ..." << endl;
//                    msgRecvQueue.push_back(i);
//                }
//                else{
//                    cout << "inMsgRecvQueue ִ�У�����û���õ���ͷ..." << endl;
//                }
//                msgRecvQueue.push_back(i);
//                sbGuard1.unlock();
            }
        }

        bool outMsgLULProc(int& command){
//            std::lock_guard<std::mutex> sbGuard(my_mutex); //����lock_guard �Ͳ�����unlock��lock��
            unique_lock<std::mutex> sbGuard1(my_mutex);

//            std::chrono::milliseconds dura(2000);      // 20s
//            std::this_thread::sleep_for(dura);              // �߳���Ϣ20s ��ص�������һ���߳�Ҳ��ȴ�
            if(!msgRecvQueue.empty()){
                // ��Ϣ���в�Ϊ��
                int command = msgRecvQueue.front();// ���ص�һ��Ԫ�أ������Ԫ���Ƿ����
                msgRecvQueue.pop_front();
                return true;
            }
            return false;
        }

        // �����ݴ���Ϣ������ȡ�����߳�
        void outMsgRecvQueue(){
            int command = 0;
            for(int i = 0; i< 100000; ++i){
                bool result = outMsgLULProc(command);
                if(result){
                    cout << "outMsgRecvQueue()ִ�С�ȡ��һ��Ԫ��" << command << endl;
                }
                else cout << "outMsgRecvQueue()ִ�С���Ϣ����Ϊ��" << i << endl;
            }
            cout << "end " << endl;
        }
    private:
        std::list<int> msgRecvQueue;
        std::mutex my_mutex;
        std::mutex my_mutex1;
        std::mutex my_mutex2;
    };
}

/**
 * �������ģʽ����
 * �̰߳�ȫ
 * call_once()
 */
namespace s4{
    // ���߳������
    namespace edition1 {
        class MyCAS {
        private:
            MyCAS() {} //˽�л����캯��
        private:
            static MyCAS *m_instance;
        public:
            static MyCAS *GetInstance() {
                // ������߳��²���ȫ
                if (m_instance == nullptr) {
                    // �����߳̿���ͬʱִ�е������������
                    m_instance = new MyCAS();
                    // ����һ����̬�����
                    static GarbageCollection c;
                }
                return m_instance;
            }

            void func() {
                cout << "test" << endl;
            }

            // ����һ���࣬��new�Ķ�����л���
            class GarbageCollection {
            public:
                ~GarbageCollection() {
                    if (MyCAS::m_instance) {
                        // �����߳̿���ͬʱ
                        delete MyCAS::m_instance;
                        MyCAS::m_instance == nullptr;
                    }
                }
            };
        };
    }
    // �����汾
    namespace edition2{
        class MyCAS{
        private:
            MyCAS(){} //˽�л����캯��
        private:
            static MyCAS* m_instance;
            static std::mutex resource_mutex;
        public:
            static MyCAS* GetInstance(){
                // ˫�ؼ��������Ч��
                if(m_instance == nullptr){
                    // ����
                    std::unique_lock<std::mutex> sbGuard(MyCAS::resource_mutex);
                    if(m_instance == nullptr){
                        m_instance = new MyCAS();
                        // ����һ����̬�����
                        static GarbageCollection c;
                    }
                }
                return m_instance;
            }
            void func(){
                cout << "test" << endl;
            }
            // ����һ���࣬��new�Ķ�����л���
            class GarbageCollection{
            public:
                ~GarbageCollection(){
                    if(MyCAS::m_instance){
                        // �����߳̿���ͬʱ
                        delete MyCAS::m_instance;
                        MyCAS::m_instance == nullptr;
                    }
                }
            };
        };
        MyCAS* MyCAS::m_instance = nullptr;
        std::mutex MyCAS::resource_mutex;
    }

    namespace callOnce{
        class MyCAS{
        private:
            MyCAS(){} //˽�л����캯��
        private:
            static MyCAS* m_instance;
            static std::mutex resource_mutex;
            static std::once_flag g_flag;
            static void CreateInstance(){
                cout << "createInstance()��ִ����" << endl;
                m_instance = new MyCAS();
                static GarbageCollection c;
            }
        public:
            static MyCAS* GetInstance(){
                // ��������߳�ͬʱִ�е��������һ���߳�ִ��CreateInstance��Ϻ󣬲Ż�ִ����һ���߳�
                std::call_once(g_flag,CreateInstance);
                return m_instance;
            }
            void func(){
                cout << "test" << endl;
            }
            // ����һ���࣬��new�Ķ�����л���
            class GarbageCollection{
            public:
                ~GarbageCollection(){
                    if(MyCAS::m_instance){
                        // �����߳̿���ͬʱ
                        delete MyCAS::m_instance;
                        MyCAS::m_instance == nullptr;
                    }
                }
            };
        };
        MyCAS* MyCAS::m_instance = nullptr;
        std::once_flag MyCAS::g_flag; // ϵͳ����ı��
    }

    // �߳���ں���
    void myThread(){
        cout << "�ҵ��߳̿�ʼִ����" << endl;
        edition2::MyCAS::GetInstance(); // �������������
        cout << "�ҵ��߳�ִ�����" << endl;
        return;
    }
    void myThread2(){
        cout << "�ҵ��߳̿�ʼִ����" << endl;
        callOnce::MyCAS::GetInstance(); // �������������
        cout << "�ҵ��߳�ִ�����" << endl;
        return;
    }
}

/**
 *  condition_variable��wait��notify_one��notify_all
 *
 *  ���Ч��
 *
 */
 // Ҫ�������ͷ�ļ�
#include <condition_variable>

namespace s5{
    class A{
    public:
        // ���յ�����Ϣ(�������)���뵽һ������
        void inMsgRecvQueue(){
            for(int i = 0; i< 100000; ++i){
                unique_lock<std::mutex> sbGuard(my_mutex);
                cout << "inMsgRecvQueue()ִ�С�����һ��Ԫ��" << i << endl;
                msgRecvQueue.push_back(i);
                // ֪ͨ����ĳһ���̣߳���wait()���ѣ�����outMsgRecvQueue������һ��������Ҫ����һ��ʱ�䣬������������wait()
                // ��ô��ʱ notify_one() ����û��Ч�������п�������һ���߳�һֱ��û�л�����������п��ܳ��ֵ�����
//                my_cond.notify_one();
                my_cond.notify_all(); // �������������߳�
            }
        }

        // �õ�֪ͨ��ʱ�� ���ǲ�ȥȡ����
//        bool outMsgLULProc(int& command){
//
//            if(!msgRecvQueue.empty()){
//                std::unique_lock<std::mutex> sbGuard(my_mutex);
//                if(!msgRecvQueue.empty()){
//                    // ��Ϣ���в�Ϊ��
//                    int command = msgRecvQueue.front();// ���ص�һ��Ԫ�أ������Ԫ���Ƿ����
//                    msgRecvQueue.pop_front();
//                    return true;
//                }
//                return false;
//            }
//        }
        // �����ݴ���Ϣ������ȡ�����߳�
        void outMsgRecvQueue(){
            int command = 0;
            while(true){
                std::unique_lock<std::mutex> sbGuard(my_mutex);
                // wait ��һ��������
                // ����ڶ��������ķ���ֵΪfalse����ôwait�������������������������У�
                        // ������ĳ���̵߳���notify_one()��Ա����Ϊֹ
                // ����ڶ�����������ֵΪtrue��wait()ֱ�ӷ���
                // ���û�еڶ����������Ǿͺ͵ڶ�����Ϊfalse���ƶ�����ĳ���̵߳���notify_one()��Ա����Ϊֹ

                // �������߳���notify_one() ����wait() ���Ѻ�wait�Ϳ�ʼ�ָ��ɻ
                // a):wait() �᳢���ٴλ�ȡ�������������ȡ���� ���̾ͻῨ����ȴ���ȡ�������ȡ�� ���ͼ���ִ��b
                // b):1�����wait�еڶ������� ���ж�������ʽ��������ʽ����false����wait�ֶԻ������������ٴ�����
                //    2������ڶ�����������ֵΪtrue����ô�ͻ�wait�ͷ��أ����������ˣ����������Ǳ���ס��
                //    3�����û�еڶ��������� wait���أ�����������
                my_cond.wait(sbGuard,[this]{
                    if(!msgRecvQueue.empty()) return true;
                    return false;
                });
                // �����ߵ������� ������һ������ס��,������һ��Ԫ��
                command = msgRecvQueue.front();
                msgRecvQueue.pop_front();
                cout << "outMsgRecvQueue()ִ�С�ȡ��һ��Ԫ��" << command << "�߳�id��" << std::this_thread::get_id() << endl;
                sbGuard.unlock();

                //
                //����������...
            }
//            for(int i = 0; i< 100000; ++i){
//                bool result = outMsgLULProc(command);
//                if(result){
//                    cout << "outMsgRecvQueue()ִ�С�ȡ��һ��Ԫ��" << command << endl;
//                }
//                else cout << "outMsgRecvQueue()ִ�С���Ϣ����Ϊ��" << i << endl;
//            }
//            cout << "end " << endl;
        }
    private:
        std::list<int> msgRecvQueue;
        std::mutex my_mutex;
        std::condition_variable my_cond;  // ������������
    };
}

/**
 * async��future��packaged_task��promise
 */

#include <future>

namespace s6{
    class A{
    public:
        int myThread(int myVar){
            cout << myVar << endl;
            cout << "myThread start " << "thread id: " << std::this_thread::get_id() << endl;
            std::chrono::milliseconds dura(5000); // ��Ϣ 5 s
            std::this_thread::sleep_for(dura);
            cout << "myThread end " << "thread id: " << std::this_thread::get_id() << endl;
            return 4;
        }
    };
    int myThread(){
        cout << "myThread start " << "thread id: " << std::this_thread::get_id() << endl;
        std::chrono::milliseconds dura(5000); // ��Ϣ 5 s
        std::this_thread::sleep_for(dura);
        cout << "myThread end " << "thread id: " << std::this_thread::get_id() << endl;
        return 5;
    }

    int myThread2(int myVar){
        cout << myVar << endl;
        cout << "myThread start " << "thread id: " << std::this_thread::get_id() << endl;
        std::chrono::milliseconds dura(5000); // ��Ϣ 5 s
        std::this_thread::sleep_for(dura);
        cout << "myThread end " << "thread id: " << std::this_thread::get_id() << endl;
        return 3;
    }

    void myThread3(std::promise<int>& temp,int calc){
        //��һϵ�и��ӵĲ���
        calc++;
        calc*=10;
        std::chrono::milliseconds dura(5000); // ��Ϣ 5 s
        std::this_thread::sleep_for(dura);

        //��������
        int result = calc;
        // ������
        temp.set_value(result); // ���������temp��
        return;
    }

    void myThread4(std::future<int>& tmpf){
        auto result = tmpf.get();
        cout << "mythread4 result " << result << endl;
    }
}

#include <atomic>

/**
 * future������Ա������shared_future��atomic
 */

namespace s7{
    int g_mycout = 0;
    std::mutex my_mutex;
    std::atomic<int> g_atomic(0);  // ��װ��һ������Ϊint��ԭ�Ӷ���
    std::atomic<bool> g_ifend = { false}; // �߳��˳����
    void myThreadAtomicBool(){
        std::chrono::seconds dura(1);
        while(!g_ifend){
            // �߳�û�˳�
            cout << "thread bool id "  << this_thread::get_id() << "start"<< endl;
            std::this_thread::sleep_for(dura);
        }
        cout << "thread bool id "  << this_thread::get_id() << "end" << endl;
    }

    void myThreadAtomic(){
        for(int i= 0;i < 1000000; ++i){
//            my_mutex.lock();
            g_mycout++;         // ��ͨ����
            g_atomic++;         // ԭ�ӱ�����Ч�ʱȼ�������Ч�ʸ�
//            my_mutex.unlock();
        }
        return;
    }
    int myThread(){
        cout << "myThread start " << "thread id: " << std::this_thread::get_id() << endl;
        std::chrono::milliseconds dura(5000); // ��Ϣ 5 s
        std::this_thread::sleep_for(dura);
        cout << "myThread end " << "thread id: " << std::this_thread::get_id() << endl;
        return 0;
    }
    int myThread2(int temp){
        cout << temp;
        cout << "myThread2 start " << "thread id: " << std::this_thread::get_id() << endl;
        std::chrono::milliseconds dura(5000); // ��Ϣ 5 s
        std::this_thread::sleep_for(dura);
        cout << "myThread2 end " << "thread id: " << std::this_thread::get_id() << endl;
        return 0;
    }
    void myThread3(std::future<int>& tmpf){
        auto result = tmpf.get(); //�ڶ��λ��쳣����Ϊget() ��һ���ƶ�����
        cout << "mythread3 result " << result << endl;
    }
    void myThread4(std::shared_future<int>& tmpf){
        auto result = tmpf.get(); //�ڶ��λ��쳣����Ϊget() ��һ���ƶ�����
        cout << "mythread3 result " << result << endl;
    }
}

/**
 * ʮһ��atomic ��̸��async ����̸
 */
namespace s8{
    int g_mycout = 0;
    std::mutex my_mutex;
    std::atomic<int> g_atomic(0);  // ��װ��һ������Ϊint��ԭ�Ӷ���
    std::atomic<bool> g_ifend = { false}; // �߳��˳����
    void myThreadAtomicBool(){
        std::chrono::seconds dura(1);
        while(!g_ifend){
            // �߳�û�˳�
            cout << "thread bool id "  << this_thread::get_id() << "start"<< endl;
            std::this_thread::sleep_for(dura);
        }
        cout << "thread bool id "  << this_thread::get_id() << "end" << endl;
    }

    void myThreadAtomic(){
        for(int i= 0;i < 1000000; ++i){
//            my_mutex.lock();
            g_mycout++;         // ��ͨ����
            g_atomic++;         // ԭ�ӱ�����Ч�ʱȼ�������Ч�ʸ�
            g_atomic+=1;        // ԭ�ӱ�����Ч�ʱȼ�������Ч�ʸ�
            g_atomic= g_atomic + 1;     // ����ԭ�Ӳ���
//            my_mutex.unlock();
        }
        return;
    }

    int myThread(){
        cout << "myThread() start " << "thread id :" << this_thread::get_id() << endl;
        return 8;
    }
}


#include <windows.h>

//#define __WINDOWSJQ_
/**
 * ʮ����windows �ٽ�������������mutex������
 */
namespace s9{
    //  �����Զ��ͷ�windows���ٽ�������ֹ�����뿪�ٽ���
    // RAII (Resource Acquisition is initialization) ��Դ��ȡ����ʼ��

    class CWinLock{
    public:
        CWinLock(CRITICAL_SECTION *pCritmp){
            m_pWinsec = pCritmp;
            EnterCriticalSection(m_pWinsec);
        }
        ~CWinLock(){
            LeaveCriticalSection(m_pWinsec);
        }
    private:
        CRITICAL_SECTION *m_pWinsec;
    };

    class A{
    public:
        void inMsgRecvQueue(){
            for(int i = 0; i< 1000; ++i){
                cout << "inMsgRecvQueue()ִ�С�����һ��Ԫ��" << i << endl;
                /**
                 * ��������Ҫ����������
                 */
#ifdef __WINDOWSJQ_
                // �����ظ����ٽ��������ǽ�ȥ���ٴ� ��Ҫ�뿪���ٴ�
//                EnterCriticalSection(&my_winsec);  // �����ٽ���
//                EnterCriticalSection(&my_winsec);   // �����ٽ���
                CWinLock vLock(&my_winsec);         // windows �µ��Զ��ͷ���
                CWinLock vLock1(&my_winsec);         // Ҳ���Խ����
                msgRecvQueue.push_back(i);
//                LeaveCriticalSection(&my_winsec);   // �뿪�ٽ���
//                LeaveCriticalSection(&my_winsec);   // �뿪�ٽ���
#else
//                my_recu_mutex.lock();
//                my_mutex.lock();
                // �����ϵ
//                testFunc1();
                // ��ʱ�����÷�
                std::chrono::milliseconds timeout(100);
//                my_mutex.try_lock_for(timeout)
                if(my_mutex.try_lock_until(std::chrono::steady_clock::now() + timeout)){   // �ȴ�100ms ���Ի�ȡ��
                    // �õ�����
                    msgRecvQueue.push_back(i);
                    my_mutex.unlock();
                }
                else{
                    cout << "û�õ���ͷ" << endl;
                    std::chrono::milliseconds timeout(100);
                    std::this_thread::sleep_for(timeout);
                }
                // ��������ֶ����
//                my_mutex.unlock();
//                my_recu_mutex.unlock();
#endif
            }
        }

        bool outMsgLULProc(int& command){
#ifdef __WINDOWSJQ_
            EnterCriticalSection(&my_winsec);
            if(!msgRecvQueue.empty()){
                // ��Ϣ���в�Ϊ��
                int command = msgRecvQueue.front();// ���ص�һ��Ԫ�أ������Ԫ���Ƿ����
                msgRecvQueue.pop_front();
                LeaveCriticalSection(&my_winsec); //���������
                return true;
            }
            LeaveCriticalSection(&my_winsec);
#else
//            unique_lock<std::mutex> sbGuard1(my_mutex);
//            unique_lock<std::recursive_mutex> sbGuard1(my_recu_mutex);
            unique_lock<std::timed_mutex> sbGuard1(my_mutex);
            std::chrono::milliseconds timeout(10000000);
            std::this_thread::sleep_for(timeout);
            if(!msgRecvQueue.empty()){
                // ��Ϣ���в�Ϊ��
                int command = msgRecvQueue.front();// ���ص�һ��Ԫ�أ������Ԫ���Ƿ����
                msgRecvQueue.pop_front();
                return true;
            }
#endif
            return false;
        }

        // �����ݴ���Ϣ������ȡ�����߳�
        void outMsgRecvQueue(){
            int command = 0;
            for(int i = 0; i< 1000; ++i){
                bool result = outMsgLULProc(command);
                if(result){
                    cout << "outMsgRecvQueue()ִ�С�ȡ��һ��Ԫ��" << command << endl;
                }
                else cout << "outMsgRecvQueue()ִ�С���Ϣ����Ϊ��" << i << endl;
            }
            cout << "end " << endl;
        }

        A(){
#ifdef __WINDOWSJQ_
            InitializeCriticalSection(&my_winsec);
#endif
        }

        /**
         * ������һ�ֳ�������testFunc1 ��Ҫ���� testFunc2
         */

        void testFunc1(){
            std::lock_guard<std::recursive_mutex> sbGuard(my_recu_mutex);
            testFunc2();
        }

        void testFunc2(){
            std::lock_guard<std::recursive_mutex> sbGuard(my_recu_mutex);
        }
    private:
        std::list<int> msgRecvQueue;
//        std::mutex my_mutex;
        // �ݹ黥����
        std::recursive_mutex my_recu_mutex;
        std::timed_mutex my_mutex;
//        std::recursive_timed_mutex my_recu_mutex


#ifdef __WINDOWSJQ_
        CRITICAL_SECTION my_winsec;  // windows �ٽ�������֮ǰ�����ȳ�ʼ��
#endif
    };

}

/**
 * ʮ��������֪ʶ���̳߳�ǳ̸������̸���ܽ�
 *
 */
namespace s10{
    class A{
    public:
        // ���յ�����Ϣ(�������)���뵽һ������
        void inMsgRecvQueue(){
            for(int i = 0; i< 100000; ++i){
                unique_lock<std::mutex> sbGuard(my_mutex);
                cout << "inMsgRecvQueue()ִ�С�����һ��Ԫ��" << i << endl;
                msgRecvQueue.push_back(i);
                // ֪ͨ����ĳһ���̣߳���wait()���ѣ�����outMsgRecvQueue������һ��������Ҫ����һ��ʱ�䣬������������wait()
                // ��ô��ʱ notify_one() ����û��Ч�������п�������һ���߳�һֱ��û�л�����������п��ܳ��ֵ�����
                my_cond.notify_one();
                my_cond.notify_one();
//                my_cond.notify_all(); // �������������߳�
            }
        }

        // �õ�֪ͨ��ʱ�� ���ǲ�ȥȡ����
//        bool outMsgLULProc(int& command){
//
//            if(!msgRecvQueue.empty()){
//                std::unique_lock<std::mutex> sbGuard(my_mutex);
//                if(!msgRecvQueue.empty()){
//                    // ��Ϣ���в�Ϊ��
//                    int command = msgRecvQueue.front();// ���ص�һ��Ԫ�أ������Ԫ���Ƿ����
//                    msgRecvQueue.pop_front();
//                    return true;
//                }
//                return false;
//            }
//        }
        // �����ݴ���Ϣ������ȡ�����߳�
        void outMsgRecvQueue(){
            int command = 0;
            while(true){
                std::unique_lock<std::mutex> sbGuard(my_mutex);
                // wait ��һ��������
                // ����ڶ��������ķ���ֵΪfalse����ôwait�������������������������У�
                // ������ĳ���̵߳���notify_one()��Ա����Ϊֹ
                // ����ڶ�����������ֵΪtrue��wait()ֱ�ӷ���
                // ���û�еڶ����������Ǿͺ͵ڶ�����Ϊfalse���ƶ�����ĳ���̵߳���notify_one()��Ա����Ϊֹ

                // �������߳���notify_one() ����wait() ���Ѻ�wait�Ϳ�ʼ�ָ��ɻ
                // a):wait() �᳢���ٴλ�ȡ�������������ȡ���� ���̾ͻῨ����ȴ���ȡ�������ȡ�� ���ͼ���ִ��b
                // b):1�����wait�еڶ������� ���ж�������ʽ��������ʽ����false����wait�ֶԻ������������ٴ�����
                //    2������ڶ�����������ֵΪtrue����ô�ͻ�wait�ͷ��أ����������ˣ����������Ǳ���ס��
                //    3�����û�еڶ��������� wait���أ�����������
                my_cond.wait(sbGuard,[this]{
                    // �����ٻ���
                    if(!msgRecvQueue.empty()) return true;
                    return false;
                });
                // �����ߵ������� ������һ������ס��,������һ��Ԫ��
                command = msgRecvQueue.front();
                msgRecvQueue.pop_front();
                cout << "outMsgRecvQueue()ִ�С�ȡ��һ��Ԫ��" << command << "�߳�id��" << std::this_thread::get_id() << endl;
                sbGuard.unlock();

                //
                //����������...
            }
//            for(int i = 0; i< 100000; ++i){
//                bool result = outMsgLULProc(command);
//                if(result){
//                    cout << "outMsgRecvQueue()ִ�С�ȡ��һ��Ԫ��" << command << endl;
//                }
//                else cout << "outMsgRecvQueue()ִ�С���Ϣ����Ϊ��" << i << endl;
//            }
//            cout << "end " << endl;
        }
    private:
        std::list<int> msgRecvQueue;
        std::mutex my_mutex;
        std::condition_variable my_cond;  // ������������
    };
}

int main(){

    /**
     * һ������˫�̶߳�д ��Ӧ�����ռ� s1
     *
     */
#if 0

    s1::A myObja;
    thread myOutMsgObj(&s1::A::outMsgRecvQueue,&myObja); // �ڶ������������� �ſ��Ա�֤�߳�����õ���һ������
    thread myInMsgObj(&s1::A::inMsgRecvQueue,&myObja);
    myOutMsgObj.join();
    myInMsgObj.join();

#endif

    /**
     *  ��Ӧ �����ռ� s2
     * ���������������ݡ�����ʱ���ô���ѹ���������ס���������ݡ�����
     *      ����������������ݵĲ������̱߳���ȴ�����
     *  1����������mutex����������
     *     ��������һ�����������һ����������̳߳�����lock()��Ա�������������ͷ��
     *     ֻ��һ���߳��������ɹ����ɹ��ı�־��lock()������,����һֱ����)
     *     �������������ݲ���Ҳ���١����˲��ܴﵽ����Ч��������Ч�ʵ���
     *
     *  2�� lock(),unlock(): ��lock,�����������ݣ�Ȼ��unlock()
     *      lock()�� unlock(),Ҫ�ɶ�ʹ�ã���������Ų�
     *      Ϊ�˷�ֹ����unlock(),����std::lock_guard����ģ�壬���� unlock()
     *  3�� lock_guard ȡ��lock() ��unlock()
     *      ʵ��ԭ���캯��ִ��lock����������ִ��unlock
     *
     *  4�� ���������ڵȶ�������������ǳ�������Ҫ�Ӷ������ʱ��
     *      cpp�У�������(����������)������ ������
     *      ���ֵ����ӣ������߳� A��B
     *      1���߳�Aִ�е�ʱ�� �Ȱѽ�����ס���ѽ���lock()��Ȼ����׼��ȥlock()����
     *      2���߳�Bִ���ˣ��Ȱ�������ס��������lock()��Ȼ����׼��ȥlock() ����
     *      ��ʱ�����ͷ�����
     *      �����Ľ��������һ�����˳��һ���Ͳ����������
     *  5��std::lock()����ģ��
     *      ������һ����ס���������������ϵĻ��������������������˲��ޣ�
     *      �������ڲ����������⣺
     *      std::lock() �����������һ��û��ס��������������ţ������л���������ס�����������ߣ��������ס���ͻ���Լ���ס���ͷŵ�
     *      Ҫô����������������Ҫô������ס
     *
     *  6��std::lock_guard : ���ڶ����Ҳ����Ҫ�Լ�unlock
     */
#if 0
    s2::A myObja;
    thread myOutMsgObj(&s2::A::outMsgRecvQueue,&myObja); // �ڶ������������� �ſ��Ա�֤�߳�����õ���һ������
    thread myInMsgObj(&s2::A::inMsgRecvQueue,&myObja);
    myOutMsgObj.join();
    myInMsgObj.join();
#endif


    /**
     * ��Ӧ����ռ� s3
     * 1�� unique_lock ȡ��lock_guard
     *    unique_lock ��һ����ģ�� ������һ��lock_guard(�Ƽ�ʹ��)
     *    unique_lock ����Ч���ϲ�һ�㡢�ڴ�ռ�ö�һ�㣬���ⲻ�Ǻܴ�
     *
     *
     *
     * 2��unique_lock �ڶ�����
     *     unique_lock ��֧��һЩlock_guard ��֧�ֵı�־
     *     1��std::adopt_lock : ��ʾ����������Ѿ���lock�ˣ��������ǰlock�������쳣����֪ͨlock
     *     2��std::try_to_lock һ���߳̿�ס20s ��һ���߳�Ҳ�Ῠס������unique_lock �ڶ������� try_to_lock ����������
     *          ���ǻ᳢����mutex��lock(),ȥ��ס���mutex �����û�������ɹ�����Ҳ���������ء�����������������
     *          �����try_to_lcok  ֮ǰ������lock �������ס����
     *     3��std::defer_lock �ڶ�����
     *          ǰ�᣺������lock(),������쳣,defer_lock ��ʾû�и�mutex��������ʼ����һ��û�м����� mutex�������Ϳ��Ե���
     *          unique_lock ����Ҫ��Ա����
     * 3����Ա����
     *      1��lock()
     *      2��unlock()  ��һЩ�ǹ������Ҫ���� ������unlock() Ȼ����lock()
     *      3��try_lock() ����ò����� ����false �õ��� ����true  ��defer_lock ����ʹ��
     *      4��realease() �������������mutexָ�� ���ͷ�����Ȩ ���unique_lock ��mutex �����й�ϵ
     *          �����unlock() ��һ��������unique_lock �� mutex�İ󶨹�ϵ��֮���Ҫ�Լ�������
     *          ���֮ǰ�Ǽ��� ����Ҫ�ֶ�����
     *          ����ֵ ��ԭ���Ļ�������ָ��
     *
     * 4��unique_lock ����Ȩ�Ĵ���
     *      1��һ��mutex Ӧ��ֻ��һ��unique_lock ����һ�����ʱ��unique_lock ��ӵ�� һ��mutex������Ȩ
     *          ����ת�Ƹ�������unique_lock ����
     *          unique_lock�������mutex������Ȩ���ܸ��� mutexҲû�и��ƹ��캯��
     *         �ƶ����� ���ʱ���ֱ�Ӱ�����Ȩת�Ƶ���sbGuard2��
     *        unique_lock<std::mutex> sbGuard1(my_mutex);
              unique_lock<std::mutex> sbGuard2(std::move(sbGuard1));
           2��Ҳ������Ϊ����ֵ ��
                unique_lock<std::mutex> sbGuard2 = rtn_unique_lock();


     *
     */
#if 0
    s3::A myObja;
    thread myOutMsgObj(&s3::A::outMsgRecvQueue,&myObja); // �ڶ������������� �ſ��Ա�֤�߳�����õ���һ������
    thread myInMsgObj(&s3::A::inMsgRecvQueue,&myObja);
    myOutMsgObj.join();
    myInMsgObj.join();

#endif

    /**
     *  �������ģʽ ���� ��Ӧ����ռ� s4
     *    1�����ģʽ���̸��
     *          һЩ�����д�����Ƚ����⣬�ͳ���д����̫һ������������ά���������ܷ��㣬���Ǳ��˽ӹܺ��Ķ�������ʹ��
     *          �����ģʽ����д�����Ĵ���ܻ�ɬ��head first����Ϊ��Ӧ���ر�����Ŀ������Ŀ�Ŀ������顢ģ�黮�־��飬�ܽ�
     *          ����� �� ���ģʽ �������ģʽ���й��������ų��������ģʽ���ף�һ��СС����Ŀ��û��Ҫ�����ģʽ����ĩ����
     *          ���Ѽ򵥵�д�ɸ��ӵģ��������ã�
     *    2���������ģʽ��������Ŀ�У���ĳ������ĳЩ������࣬���ڸ���Ķ�����ֻ�ܴ���һ���������Ҵ�������
     *          ʹ��Ƶ�ʸ�
     *    3���������ģʽ�����̰߳�ȫ����������
     *          ��Ҫ�������Լ��������̣߳����������̣߳�����MyCAS���������Ķ��������߳̿��ܻ���ֻһ��
     *          GetInstance() Ҫ����
     *    4��std::call_once():����ģ�� C++11 ����� �ڶ���������һ��������
     *          ���ܣ���֤����ֻ������һ�� ���߱���������������Ч���ϱȻ��������ĵ���Դ����
     *          call_once() ��Ҫ��һ����ǽ��ʹ�� std::once_flag,
     *          call_once() ͨ����Ǿ�����Ӧ�ĺ����Ƿ�ִ�У� ����call_once�ɹ������Ͱ�std::once_flag ����Ϊһ���ѵ���
     *          ״̬
     */
#if 0
//    s4::edition1::MyCAS* a = s4::edition1::MyCAS::GetInstance();
//    s4::edition1::MyCAS* b = s4::edition1::MyCAS::GetInstance();
//    a->func();
//    b->func();
    // �����߳���ں�����ͬ,������ͨ· ͬʱִ�����getInstance(),�������
//    std::thread myObj1(s4::myThread);
//    std::thread myObj2(s4::myThread);
//    myObj1.join();
//    myObj2.join();
        // call_once
    std::thread myObj1(s4::myThread2);
    std::thread myObj2(s4::myThread2);
    myObj1.join();
    myObj2.join();

#endif

    /**
     * 1������������condition_variable��wait()��notify_one()��notify_all
     * 2��   �߳� A���ȴ�һ����������
     *       �߳� B��ר������Ϣ����������Ϣ
     * 3��   condition_variable ʵ������һ���࣬��һ����������ص��࣬˵���˾��ǵȴ�һ���������
     *       �������Ҫ�ͻ�������Ϲ���
     * 4�� ������������˼��
     * 5�� notify_all()
     */

#if 0

    s5::A myObja;
    thread myOutMsgObj(&s5::A::outMsgRecvQueue,&myObja); // �ڶ������������� �ſ��Ա�֤�߳�����õ���һ������
    thread myOutMsgObj2(&s5::A::outMsgRecvQueue,&myObja); // Ϊ����ʾ notify_all()
    thread myInMsgObj(&s5::A::inMsgRecvQueue,&myObja);
    myOutMsgObj.join();
    myOutMsgObj2.join();
    myInMsgObj.join();

#endif
    /**
     * һ�� async��future��packaged_task��promise
     * 1��ϣ���̷߳���һ�������std::async,std::future
     *      std::async ��һ������ģ�壬����һ���첽���� ����һ��std::future����ģ�壩 ����
     *      std::async �����Զ�����һ���̣߳�����ʼִ����Ӧ���߳���ں�����
     *      ����ֵstd::future������������߳���ں��������صĽ���������̷߳��صĽ����
     *      ����ͨ��future����ĳ�Ա����get(), ��ȡ�����std::future�ṩ��һ�ַ����첽��������Ļ���
     *      ������������ܲ��������õ��������ڲ��õĽ������̺߳���ִ����ϵ�ʱ��future����ᱣ��һ��ֵ��
     *      get()����ֻ�ܵ���һ�Σ������������ö�Σ���������쳣������Ӧ��ʹ��get() ����wait() �ȴ����߳̽���
     *      ���û��get() �����ȴ����߳�ִ���� ���˳������̻߳��ߵ����ȴ������˳�
     *      ͨ��������std::async() ����һ������ �������� Ϊstd::launch���ͣ�ö�����ͣ�
     *      a) std::launch::deferred ��ʾ�߳���ں������ñ��ӳٵ�std::future��wait()����get()�������ò�ִ��
     *          ���wait����getû�б����� �̻߳�ִ���𣬴𣺲�������ִ�У��߳�ʵ��û�д���
     *      b) std::launch::async ��ʾ����async������ʱ�� �Ϳ�ʼ�����߳�
     * 2��package_task ���������ģ�壩
     *      ģ����� �ǿɵ��ö���ͨ��package_task���԰Ѹ��ֿɵ��ö����װ���������㽫����Ϊ�߳���ں���
     *      package_task��װ�����Ŀɵ��ö��󻹿���ֱ�ӵ��ã�Ҳ����˵������Ҳ�ǿ��Ե���
     *
     * 3��std::promise ��ģ��
     *      �����ܹ���ĳ���߳��и�����ֵ���������߳��� �����ֵȡ������,ʵ���߳�֮���ͨ��
     */
#if 0
    s6::A a;
    int tmp = 12;
    cout << "hello world" << "main thread id: " << std::this_thread::get_id() <<endl;
    // �Զ�����һ���̲߳�ִ�У�result ��һ������ֵ
//    std::future<int> result = std::async(s6::myThread);
    // �Գ�Ա������Ϊ�߳���ڣ��ڶ��������õ����� �ű�ס�߳�����һ������ �������û�д����̣߳���Ϊ�߳�id��һ����
//    std::future<int> result = std::async(&s6::A::myThread, &a, tmp);
    // ��ʾ�ӳٵ����̺߳��� ֱ��get����wait����
//    std::future<int> result = std::async(std::launch::deferred,&s6::A::myThread, &a, tmp);
    //    ��ʾ����async������ʱ�� �Ϳ�ʼ�����߳�,����ִ���̺߳���
//    std::future<int> result = std::async(std::launch::async,&s6::A::myThread, &a, tmp);
//    cout << "continue.......!" << endl;
//    int def = 0;
    // result.get() ����Ῠס �ȴ��̺߳���ִ����� �õ����
//    result.wait(); // �ȴ��̷߳��أ����ǲ����ؽ�� ������join()
//    cout << result.get() << endl;
//    cout << result.get() << endl;  // ��������������get()

    /**
     * 1��packaged_task ��װ�ɵ��ö���
     */
//    std::packaged_task<int(int)> mypt(s6::myThread2);  // �Ѻ���s6::myThread2() ��װ����
//    // �ɵ��ö��󡢲���
//    std::thread t1(std::ref(mypt),1);  // �߳�ֱ�ӿ�ʼִ�� ��1 ����Ϊ�߳���ں����Ĳ���
//    t1.join();
//    // ��ȡ���ؽ��
//    std::future<int> result = mypt.get_future();
//    cout << result.get() << endl;

    /**
     * 2��packaged_task ��װlambda ���ʽ
     */

//    function<int(int)> ll;
//    ll = [](int x) -> int {
//        cout << x << endl;
//        cout << "myThread start " << "thread id: " << std::this_thread::get_id() << endl;
//        std::chrono::milliseconds dura(5000); // ��Ϣ 5 s
//        std::this_thread::sleep_for(dura);
//        cout << "myThread end " << "thread id: " << std::this_thread::get_id() << endl;
//        return 2;
//    };
//    std::packaged_task<int(int)> mypt2(std::ref(ll));

//    std::thread t2(std::ref(mypt2),1);  // �߳�ֱ�ӿ�ʼִ�� ��1 ����Ϊ�߳���ں����Ĳ���
//    t2.join();
    // ��ȡ���ؽ��
//    std::future<int> result2 = mypt2.get_future();
//    cout << result2.get() << endl;
    /**
     * 3��packaged_task ����ֱ�ӵ���
     */
//    mypt2(20);  // ֱ�ӵ��ã��൱�ں�������
//    std::future<int> result3 = mypt2.get_future();
//    cout << result3.get() << endl;

/**
 * ����packaged_task
 */
//    vector<packaged_task<int(int)>> myTasks;
//    // �Ѷ��� �Ƶ�������
//    myTasks.push_back(std::move(mypt2));
//    std::packaged_task<int(int)> mypt3;
//    auto iter = myTasks.begin();
//    mypt3 = std::move(*iter);
//    myTasks.erase(iter);// ������ʧЧ,�Ƴ���������һ��
//    mypt3(10);
//    std::future<int> result = mypt3.get_future();
//    cout << result.get() << endl;
/**
 *   4��std::promise
 */
    std::promise<int> myProm;  // ����һ�����󣬱���һ��int
    std::thread t1(s6::myThread3,std::ref(myProm),180);
    t1.join(); // ����㲻�� �ᱨ�쳣

    //��ȡ���ֵ
    std::future<int> result = myProm.get_future();
//    cout  << result.get() << endl;
    std::thread t2(s6::myThread4,std::ref(result));
    t2.join();

#endif

    /**
     * һ�� future������Ա������shared_future��atomic
     * 1�� future������Ա������std::future_status status wait_for()
     * 2�� share_future ����ģ�壩�������̶߳���õ����������get() �Ǹ���
     * 3��atomic ԭ�Ӳ�����
     *      �����������̱߳���У������������ݣ������������������ݣ�����
     *      �������̶߳�һ���������в���atomVal���������һ���̶߳���һ���߳�д����ʹ����Ҳ�������
     *      ԭ�Ӳ�������ҪŶ�õ������������������������Ķ��̲߳�����̷�ʽ
     *      �ڶ��߳��У����ᱻ��ϵĳ���ִ��Ƭ�Σ�Ч�ʱȻ������ߣ�ԭ�Ӳ����ǲ��ɷָ��״̬
     *      �����������һ������Σ�������һ������Σ���ԭ�Ӳ���һ���ĳ����������
     *      std::atomic ��ģ�壬Ϊ�˷�װĳ�����͵�ֵ
     *      // ���߳�
     *      int temVal = atomVal;
     *      //д�߳�
     *      atomVal = 6;
     *
     */

#if 0
    /**
     * 1��std::future_status status wait_for()
     */
    cout << "hello world" << "main thread id: " << std::this_thread::get_id() <<endl;
//     �Զ�����һ���̲߳�ִ�У�result ��һ������ֵ
//    std::future<int> result = std::async(std::launch::async,s7::myThread);
//    std::future<int> result = std::async(std::launch::deferred,s7::myThread);
//    cout << "continue.......!" << endl;
//    std::future_status status = result.wait_for(std::chrono::seconds(6)); // �ȴ�һ��
//    if(status == std::future_status::timeout){  //��ʱ������ȴ�1s��ϣ���㷵�أ�������û�з��أ����Գ�ʱ
//        //��ʾ�̻߳�ûִ���꣬
//        cout << "��ʱ���̻߳�ûִ����" << endl;
//    }
//    else if(status == std::future_status::ready){
//        cout << " �̳߳ɹ����� " << endl;
//    }
//    else if(status == std::future_status::deferred){
//        cout << "�̱߳��ӳ�ִ��" << endl;
//        // ����̺߳����������߳�ִ�еģ��൱��û�д������߳�
//        cout << result.get() << endl;
//    }

/**
 *  2��share_future������ģ�壩�������̶߳���õ����������get() �Ǹ���
 *  ����get���
 */
//    std::packaged_task<int(int)> mypt(s7::myThread2);
//    std::thread t1(std::ref(mypt),1);
//    t1.join();
//    std::future<int> result = mypt.get_future();
//    std::shared_future<int> result_s(std::move(result));
//    bool ifcanget = result.valid();
//    std::shared_future<int> result_s(result.share());
//    std::shared_future<int> result_s(mypt.get_future()); //ͨ��get_future ����һ��shared_future ����
//    auto mythreadRes = result_s.get();
//    mythreadRes = result_s.get();
//    ifcanget = result.valid();

//    std::thread t2(s7::myThread4,std::ref(result_s));
//    t2.join();
//    cout << result_s.get() << endl;
//    cout << result_s.get() << endl;
//    cout << result_s.get() << endl;

    /**
     * 3��ԭ�Ӳ�����atomic
     */
//    hello worldmain thread id: 1
//    ���Ľ����1607245
//    over
//    �밴���������. . .
    thread myObj1(s7::myThreadAtomicBool);
    thread myObj2(s7::myThreadAtomicBool);
    std::chrono::seconds dura(5);
    std::this_thread::sleep_for(dura);
    s7::g_ifend = true;
    myObj2.join();
    myObj1.join();

    cout << "���Ľ����" << s7::g_mycout << endl;
    cout << "���Ľ����" << s7::g_atomic << endl;

#endif

    /**
     * atomic ��̸��async ����̸
     *  1�� atomic ��� ++��--��+=��&=��|=��^=��һԪ�ǿ��Եģ����Ƕ���Щ��������ԭ�Ӳ���������Ҫע��
     *
     *  2��async
     *      ����������async��������һ�첽����һ�㲻�д���һ���߳�
     *      std::thread() ���ϵͳ��Դ���ţ���ô���ܴ����߳̾ͻ�ʧ�ܣ���ôִ��std::thread(),�ͻᵼ������������ܱ���
     *      ���������Բ�ͬ��ʱ����ʱ��async���������̣߳�������get()�����߳���ִ���߳���ں���
     *      1�������std::launch::defferred ��Ϊ��һ����
     *          û�д����̣߳������ڵ���get()ʱ�����߳���ִ���߳���ں��������û�е�������ں�������ִ��
     *      2�����std::launch::async ��Ϊ��һ����������봴��һ���߳�
     *      3�����ͬʱ�� std::launch::async |std::launch::defferred,
     *          ��ζ�ſ�����std::launch::async
     *             Ҳ������ std::launch::defferred
     *      4�����û��Ĭ�ϵ�һ���� Ĭ���� std::launch::async |std::launch::defferred (��ȷ��)
     *          ��ϵͳ�������о�����ͬ���������첽��ϵͳ��ξ�����
     * 3��std::thread()�� std::async() ������
     *      std::thread():һ���ᴴ���̣߳��������ʧ�ܣ�����ֱ�ӱ���������ֵ���ýӣ�Ҫ��ȫ��������package_list ����promise
     *      std::async: �����첽���񣺿��ܴ����̡߳����ܲ������̣߳�����ֵ��std::future<T> �����õ�����ֵ��һ�㲻���쳣����
     *      ��������Ϊ���ϵͳ���ţ����Ӷ�������ĵ��û���std::launch::defferred����ô�����ᴴ���̣߳�������get() �����߳�
     *      ִ�У������ǿ��ʹ��std::launch::async �����ܻᵼ��ϵͳ����
     *
     * 4�����飺һ���������߳��������˳���100-200
     *
     * 5��std::async��ȷ��������Ľ���� std::future<int> result = std::async(s8::myThread);
     *      ���⽹�����ڣ��첽����һ���ᱻִ�У�Ҫ�ж���û�б��Ƴ�ִ�У�Ҫʹ��wait_for����
     *
     */

#if 0
    /**
     * 1��atomic ��̸
     */
//    thread myObj1(s8::myThreadAtomic);
//    thread myObj2(s8::myThreadAtomic);
//
//    myObj2.join();
//    myObj1.join();
//
//    cout << "���Ľ����" << s8::g_mycout << endl;
//    cout << "���Ľ����" << s8::g_atomic << endl;

    /**
     * 2��async ����̸
     * std::thread() ���ϵͳ��Դ���ţ���ô���ܴ����߳̾ͻ�ʧ�ܣ���ôִ��std::thread(),�ͻᵼ������������ܱ���
     *
     */
    std::future<int> result = std::async(s8::myThread); // ��ȷ�����
//    std::future<int> result = std::async(std::launch::async,s8::myThread);
//    std::future<int> result = std::async(std::launch::async|std::launch::deferred ,s8::myThread); // ��ȷ�����
    cout << "continue.......!" << endl;
//    std::future_status status = result.wait_for(std::chrono::seconds(0)); // �ȴ�һ��
    std::future_status status = result.wait_for(0s); // �ȴ�һ��

    if(status == std::future_status::deferred){
        cout << "�̱߳��ӳ�ִ��,û�п����߳�" << endl;
        // ����̺߳����������߳�ִ�еģ��൱��û�д������߳�,��ʱ���ֶ�ִ���̺߳���
        cout << result.get() << endl;
    }
    else if(status == std::future_status::timeout){  //��ʱ������ȴ�1s��ϣ���㷵�أ�������û�з��أ����Գ�ʱ
        //��ʾ�̻߳�ûִ���꣬
        cout << "��ʱ���̻߳�ûִ����" << endl;
    }
    else if(status == std::future_status::ready){
        cout << " �̳߳ɹ����� " << endl;
    }

#endif
    /**
     * ʮ����windows �ٽ�������������mutex������
     * 1��windows �ٽ���: ��windwos�µ��ٽ�������C++11 mutex�ǳ�����
     *
     * 2����ν����ٽ�������
     *      ��ͬһ���߳��У������ظ���ȥ��ͬ���ٽ�������������ҲҪ��Ӧ�������Σ�C++11 ������������
     * 3���Զ�����������ʵ��windows ������ٽ���
     *
     * 4��recursive_mutex �ݹ�Ķ�ռ������,����Ҫ��μ�����ʱ����õ�
     *      std::mutex ��ռ������
     *      std::recursive_mutex �ݹ��ռ������,����ͬһ���̣߳�ͬһ����������α�lock()
     *      Ҳ��lock() ��unlock() �ݹ���Դ���������
     *
     * 5������ʱ��(�ݹ�)������std::timed_mutex�� std::recursive_timed_mutex
     *      �ȴ�һ��ʱ��
     *      1��try_lock_for(),����ȴ�һ��ʱ���õ��������߳�ʱ�����������ȥ
     *      2��try_lock_until() ������һ��δ����ʱ��㣬��δ����ʱ��û����ʱ����ڣ�����õ����� ���̾�������
     *                         ���ʱ�䵽�ˣ���û�õ�����Ҳ��������
     *
     *
     */
#if 0
    s9::A myObja;
    thread myOutMsgObj(&s9::A::outMsgRecvQueue,&myObja); // �ڶ������������� �ſ��Ա�֤�߳�����õ���һ������
    thread myInMsgObj(&s9::A::inMsgRecvQueue,&myObja);
    myOutMsgObj.join();
    myInMsgObj.join();

#endif
    /**
     * ʮ��������֪ʶ���̳߳�ǳ̸������̸���ܽ�
     * 1����ٻ���
     *      wait(),notify_one(),notify_all()
     *          my_cond.wait(sbGuard,[this]{
                    // �����ٻ���
                    if(!msgRecvQueue.empty()) return true;
                    return false;
                });
        2��atomic<int> a(1);  // ����ԭ�� �Ͳ��������л�
            void thread(){
                a++;
            }
            while(true){
                cout << a << endl;   // ��atm��ԭ�Ӳ���,����������������ԭ�Ӳ���
                auto atm2 = a;       // �����ǲ��Ϸ��� û�п������캯������ֵҲ������
                atomic<int> atm2(atm.load())  // �����ǿ��Ե�,��
                auto atm2(atm.load())

                atm2.store(12)       // д
            }

     */
#if 1

    s10::A myObja;
    thread myOutMsgObj(&s10::A::outMsgRecvQueue,&myObja); // �ڶ������������� �ſ��Ա�֤�߳�����õ���һ������
    thread myInMsgObj(&s10::A::inMsgRecvQueue,&myObja);
    myOutMsgObj.join();
    myInMsgObj.join();

#endif
    cout << "main() start " << "main thread id :" << this_thread::get_id() << endl;
    cout << "over " << endl;
//    system("pause");
    return 0;
}
