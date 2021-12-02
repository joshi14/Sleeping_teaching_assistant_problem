#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <windows.h>
#include <stdbool.h>

sem_t students;
sem_t teachingAssistant;
pthread_mutex_t mutex;

int numStudents;
int numChairs = 3;
int *waitingChairs;   //circular queue
int remaining_chairs;
int front = 0; //pointers for the
int rear = 0;  //circular queue waiting chairs
bool ta_sleeping_flag = true;

void* ta_working(){
    while(1){
        if(remaining_chairs < numChairs){
            printf("Teaching Assistant is WOKEN UP by student %d\n",waitingChairs[front]);
            ta_sleeping_flag = false;
            sem_wait(&students);

            //Lock the mutex, to prevent race condition.
            pthread_mutex_lock(&mutex);

            //teaching assistant helps his student
            printf("Teaching Assistant helps Student %d\n",waitingChairs[front]);
            printf("Remaining Chairs:%d\n",++remaining_chairs);
            front = (front+1)%numChairs;
            Sleep(5000);
            printf("Student leaves the room\n");

            //unlock the chair so subsequent student will occupy.
            pthread_mutex_unlock(&mutex);
            sem_post(&teachingAssistant);
        }
        //when no students are waiting
        else if(!ta_sleeping_flag){
            printf("No Students waiting.\n");
            printf("-------------------SLEEPING-------------------------\n");
            ta_sleeping_flag = true;
        }
    }

}
bool isWaiting(int student_id){
    for(int i =0;i<numChairs;i++)
        if(waitingChairs[i] == student_id)
            return true;
    return false;
}

void* student_working(void* x){
    int student_id = *(int*)x;
    while(1){
        if(isWaiting(student_id)){
            continue;
        }
        printf("Student %d is doing assignment\n",student_id);
        Sleep(20);

        pthread_mutex_lock(&mutex);
        if(remaining_chairs > 0){
            waitingChairs[rear]=student_id;
       printf("Student %d is waiting for the teaching assistant to complete\n",student_id);
            printf("Remaining Chairs:%d\n",--remaining_chairs);
            rear = (rear + 1) % numChairs;

            pthread_mutex_unlock(&mutex);

            sem_post(&students);//signaling the student semaphore
            sem_wait(&teachingAssistant);//waking up the Teaching Assistant if sleeping.
        }
        else{
            pthread_mutex_unlock(&mutex);
            printf("No chairs available.Student %d will come later\n",student_id);
            Sleep(500);
        }

    }
}

int main(){
    //Getting user inputs for number of students and chairs
    printf("Enter number of students \n");
    scanf("%d",&numStudents);

    printf("Enter the number of chairs\n");
    scanf("%d",&numChairs);
    remaining_chairs = numChairs;
    waitingChairs = (int*)malloc(numChairs * sizeof(int));

    //Declaring Threads
    pthread_t TA;
    pthread_t student_t[numStudents];
    int student_id[numStudents];

    //initializing semaphores
    sem_init(&students,0,0);
    sem_init(&teachingAssistant,0,1);

    //Thread Creation
    pthread_mutex_init(&mutex,NULL);
    pthread_create(&TA,NULL,ta_working,NULL);
    for(int i=0;i<numStudents;i++){
        student_id[i] = i+1;
        pthread_create(&student_t[i],NULL,student_working, (void *) &student_id[i]);
    }

    //wait for the thread termination
    pthread_join(TA,NULL);
    for(int i=0;i<numStudents;i++)
        pthread_join(student_t[i],NULL);

    return 0;
}
