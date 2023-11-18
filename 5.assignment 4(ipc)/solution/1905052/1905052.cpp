#include <bits/stdc++.h>
#include <random>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

int n,m,w,x,y; // necessary input, n=total student , m=student in a group , w,x,y=time taken for task
int initial_time; // it will take the starting time 
vector<string> student_state; // it is used for storing state of the student during printing

pthread_t* students; // thread for each student
pthread_t* staff_members; // thread for each staff

vector<sem_t> student_semaphore; // sempaphore for each student
sem_t binding; // semaphore for 2 binding student

pthread_mutex_t locked; // mutex used for printing the output such that output become consistent
pthread_mutex_t stuff_locked; // mutex used for staff
pthread_mutex_t student_locked; // mutex used for student
pthread_mutex_t write_locked; // mutex used for writing in entry book

int rc=0; // it is used for counting the number of reader at a specific time
int subCount=0; // it is used for couning the submission 

void printingStation(int);
void take_ps(int); // similiar to take_forks function of Dining Philosophers problem of slide
void leave_ps(int); // similiar to put_forks function of Dining Philosophers problem of slide
void test(int); // similiar to test function of Dining Philosophers problem of slide
void bindingStation(int);
void library(int); // similiar to writer function of The Readers and Writers problem of slide
void read(int); // similiar to reader function of The Readers and Writers problem of slide

void* staffTask(void* arg)
{
    int stuff_num= *(int *)arg;
    read(stuff_num);
}

void* studentTask(void* arg)
{
    int s_num = *(int *)arg; // student id

    //add random delays before a student steps into the printing phase.
    random_device rd; // an instance of the random_device class
    default_random_engine generator(rd()); // an instance of the default_random_engine class that  takes the value produced by the random_device as a seed to initialize the random number generator
    poisson_distribution<int> p; // an instance of the poisson_distribution class
    sleep(p(generator));

    printingStation(s_num); // for printing task

    if(s_num%m==(m-1)) // group leader try to binding the paper got from groupmate and submitting the paper
    {
        int firstMember=(s_num/m)*m;
        int lastMember=firstMember+m-1;
        for(int i=firstMember;i<=lastMember;i++)
        {
            pthread_join(students[i],NULL); // group leader will wait for other groupmate to finish their printing
        }
        pthread_mutex_lock(&locked);
        cout<<"Group " << (s_num/m)+1<<" has finished printing at time "<<time(NULL)-initial_time<<endl;
        pthread_mutex_unlock(&locked);

        bindingStation(s_num);

        library(s_num);

    }
}

int main(void)
{
    freopen("input.txt", "r", stdin); // input will be taken from input.txt file
    freopen("output.txt", "w", stdout); // output will be given into output.txt file
    cin>>n>>m>>w>>x>>y; // take the input from "input.txt" file

    initial_time=time(NULL); // take the current time in initial_time variable as a starting time

    staff_members=new pthread_t[2]; // thread for each staff to execute their work
    int staff_id1=0;
    pthread_create(&staff_members[0],NULL,staffTask,(void *)&staff_id1);

    int staff_id2=1;
    pthread_create(&staff_members[1],NULL,staffTask,(void *)&staff_id2);
    
    vector<int> s(n); // it is used for Randomness in student arrival at printing stations

    for(int i=0;i<n;i++)
        s[i]=i;
    random_device rand;
    default_random_engine generator(rand());
    shuffle(s.begin(), s.end(), generator); // now s contain value from 0 to n-1 but in random order

    //initializing mutex
    pthread_mutex_init(&locked, 0);
    pthread_mutex_init(&stuff_locked, 0);
    pthread_mutex_init(&student_locked, 0);
    pthread_mutex_init(&write_locked, 0);

    //initializing semaphore
    sem_init(&binding,0,2); // initialize with 2 as there are 2 binding station and group leader use these 2 station simultaneously

    students=new pthread_t[n]; // thread for each student to execute their work

    student_state.resize(n); // it is used for storing state(string) of the student during printing
    student_semaphore.resize(n);  // semaphore for each student 
    for (int i = 0; i < n; i++)
	{
		sem_init(&student_semaphore[i],0,0); // initialize with 0
	}

    for(int i=0;i<n;i++) // creating thread for each student who execute the studentTask function
    {
        pthread_create(&students[s[i]],NULL,studentTask,(void *)&s[i]);
    }

    for(int i=0;i<n;i++)
    {
        if(i%m==(m-1)) // group leader SM
        {
            pthread_join(students[i],NULL);
        }   
    }
    return 0;
}

void printingStation(int s_num)
{
    student_state[s_num]="idle";
    pthread_mutex_lock(&locked);
    int curr_time=time(NULL);
    cout<<"Student "<<s_num+1<<" has arrived at the print station at time "<<curr_time-initial_time<<endl;
    pthread_mutex_unlock(&locked);
   
    //used dinning philosopher algorithm
    take_ps(s_num); // student is trying to use the printing station
   
    sleep(w); // time taken to printing the paper in the printing station

    leave_ps(s_num); // student is leaving the printing station

    pthread_mutex_lock(&locked);
    curr_time=time(NULL);
    cout<<"Student "<<s_num + 1<<" has finished printing at time "<<curr_time-initial_time<< endl;
    pthread_mutex_unlock(&locked);
}

void take_ps(int s_num) // student is trying to take a printing station
{
    pthread_mutex_lock(&student_locked);
	student_state[s_num] = "wait";
	test(s_num);
	pthread_mutex_unlock(&student_locked);
	sem_wait(&student_semaphore[s_num]);
}

void leave_ps(int s_num)
{
    pthread_mutex_lock(&student_locked);
    student_state[s_num] = "complete";
    int ps_num=(s_num+1)%4+1; // printing station number of student who just completed the printing
    vector<int> tmp1(n); // used for taking the list of student with prioriry
    int j=0;
    for(int i=0;i<n;i++)
    {
        int tmp_ps=(i+1)%4+1; // printing station number of i
        if(tmp_ps==ps_num && student_state[i]=="wait" && ((s_num/m)==(i/m)) && i!=s_num)
        {
            tmp1[j++]=i;
        }
    }
    for(int i=0;i<n;i++)
    {
        int tmp_ps=(i+1)%4+1; // printing station number of i
        if(tmp_ps==ps_num && student_state[i]=="wait" && !((s_num/m)==(i/m)) && i!=s_num)
        {
            tmp1[j++]=i;
        }
    }
    for(int i=0;i<j;i++)
    {
        test(tmp1[i]);
    }
    pthread_mutex_unlock(&student_locked);
}

void test(int s_num)
{
    int psFree=1; // psFree is 1 when printing station is free and 0 when printing station is occupied
    int ps_num_tmp=(s_num+1)%4+1; // assigned printing station number for the student
    for(int i=0;i<n;i++)
    {
        if (student_state[i] == "print" && ((i+1)%4+1)==ps_num_tmp) // printing station is occupied
		{
			psFree = 0;
			break;
		}
    }
    if(psFree==1&&student_state[s_num]=="wait") // printing station is free and the student want to print
    {
        student_state[s_num]="print";
        sem_post(&student_semaphore[s_num]);
    }
}

void bindingStation(int s_num)
{
    sem_wait(&binding);
    pthread_mutex_lock(&locked);
    cout<<"Group " << (s_num/m)+1<<" has started binding at time "<<time(NULL)-initial_time<<endl;
    pthread_mutex_unlock(&locked);

    sleep(x); // time taken to binding the paper in the binding station

    sem_post(&binding);

    pthread_mutex_lock(&locked);
    cout<<"Group " << (s_num/m)+1<<" has finished binding at time "<<time(NULL)-initial_time<<endl;
    pthread_mutex_unlock(&locked);
}

void library(int s_num)
{
    pthread_mutex_lock(&write_locked);

    pthread_mutex_lock(&locked);
    cout<<"Group "<<(s_num/m)+1<<" has started submitting the report at time "<<time(NULL)-initial_time<< endl;
    pthread_mutex_unlock(&locked);

    sleep(y);

    pthread_mutex_lock(&locked);
    cout<<"Group "<<(s_num/m)+1<<" has finished submitting the report at time "<<time(NULL)-initial_time<< endl;
    pthread_mutex_unlock(&locked);

    subCount++;

    pthread_mutex_unlock(&write_locked);
}

void read(int stuff_num)
{
    while(true)
    {
        pthread_mutex_lock(&stuff_locked);
        rc++;
        if(rc==1)
            pthread_mutex_lock(&write_locked);
        pthread_mutex_unlock(&stuff_locked);

        pthread_mutex_lock(&locked);
        cout<<"Staff "<<stuff_num+1<<" has started reading the entry book at time "<<time(NULL)-initial_time<<". No. of submission = "<<subCount<<endl;
        pthread_mutex_unlock(&locked);

        sleep(y);

        pthread_mutex_lock(&stuff_locked);
        rc--;
        if(rc==0)
            pthread_mutex_unlock(&write_locked);
        pthread_mutex_unlock(&stuff_locked);
        random_device rd;
        default_random_engine generator(rd());
        poisson_distribution<int> p;
        sleep(p(generator));
    }
}