#include <ros/ros.h>
#include <std_msgs/Float64.h>
#include <std_msgs/String.h>

#include <geometry_msgs/Wrench.h>

#include <math.h>


#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>  //internet domain addresses, struct hostent

//sensitve of data. 
#define sensitive_Fx 26.98
#define sensitive_Fy 28.68
#define sensitive_Fz 31.70
#define sensitive_Mx 1530.40
#define sensitive_My 1500.45
#define sensitive_Mz 1155.94

//function printf error
void error(const char *msg)
{
    perror(msg);
    exit(0);
}



//function:power x^y
float pow(int x,int y)
{
int i;
float temp=1.0;
if(y<=0)
  return 1.0;
else
   {
   for(i=0;i<y;i++)
    temp *=x;
   return temp;
}
}



int main(int argc, char **argv)
{
  int sockfd, portno, nc; //sockfd,  variables store the values returned by the socket system call and the accept system call.
    struct sockaddr_in serv_addr;
    struct hostent *server;
//struct  hostent
// {
   //    char    *h_name;        /* official name of host */
    //   char    **h_aliases;    /* alias list */
     //  int     h_addrtype;     /* host address type */
      // int     h_length;       /* length of address */
      // char    **h_addr_list;  /* list of addresses from name server */
     //  #define h_addr  h_addr_list[0]  /* address, for backward compatiblity */
  //};
  // */


    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    //Takes such a name as an argument and returns a pointer to a hostent containing information about that host
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    //The connect function is called by the client to establish a connection to the server

    printf("Please enter the message: ");

    bzero(buffer,256);
    fgets(buffer,255,stdin);
    //uses fgets to read the message from stdin
    
    nc = write(sockfd,buffer,strlen(buffer));
    //writes the message to the socket

    if (nc < 0) 
         error("ERROR writing to socket");


//ros part ======================================================================



  ros::init(argc,argv,"Forcesensor_publisher"); 	// name of this node will be ""Forcesensor_publisher"
  ros::NodeHandle n; 				// two lines to create a publisher object that can talk to ROS
  ros::Publisher my_publisher_object = n.advertise<std_msgs::String>("topic1",1);
  //"topic1" is the name of the topic to which we will publish
  // the "1" argument says to use a buffer size of 1; could make larger, if expect network backups

  ros::Publisher my_publisher_object2 = n.advertise<geometry_msgs::Wrench>("topic2",1);

  ros::Publisher my_publisher_object3 = n.advertise<geometry_msgs::Wrench>("wrench",100);

  //Wrench and String type.
  geometry_msgs::Wrench myWrench,myWrench_past,myWrench_final;

  std_msgs::String mystring;
  

  // ROS Rates
  ros::Rate loopRate(50); 	       		// Create a ros::Rate object. Set the time for a 1Hz sleeper timer.
  int i,num,temp,j,data_missing =0;

  while(ros::ok()) //work loop		        
    {
    bzero(buffer,256);
    //clear buffer to zero
    nc = read(sockfd,buffer,255);
    //read data from socket,255 is the size of the message
    if (nc < 0) 
         error("ERROR reading from socket");

    // printf("%s\n",buffer); //debug the buffer string

      mystring.data = buffer;  
      /*   
  for(i=0; i<64; i++)
   {
     if(buffer[i] == 10) //buffer[i] == '/n'
    {
      data_missing = 0;
      num=-1;
      for(j=0;j<25;j++)
      {
        if(buffer[i+j+1]>='0'&&buffer[i+j+1]<='9' || buffer[i+j+1]>='A' &&buffer[i+j+1]<='F')
	  ;
        else
	  data_missing = 1;//check if the buffer[i] to buffer[i+24] is complete
      }
      
    }
     else if (data_missing == 0)//no data missing
    {
      num++;
      if(num!=0) //num = 0 is the first charater,which should be ignore.
    {
      if(buffer[i]>='0'&&buffer[i]<='9') 
        temp = buffer[i] - '0';//tranfer ascii to decimal number
      else if(buffer[i]>='A'&&buffer[i]<='F')
        temp = buffer[i] - 'A'+10;///tranfer ascii to decimal number
      else
        temp = 0;
     
      //every data has four character,total 6 data.
      if(num/4 == 0)//first data
	  {
          if(num%4 == 1)
          myWrench.force.x = 0;
          myWrench.force.x=temp*(pow(16,3-(num-1)%4))+ myWrench.force.x;
          //raw data
	  //tranformed data    
          }
      else if(num/4 == 1)//second data
          {
          if(num%4 == 1)
            myWrench.force.y = 0;
          myWrench.force.y=temp*(pow(16,3-(num-1)%4))+ myWrench.force.y;
          }
      else if(num/4 == 2)//third data
	  {
          if(num%4 == 1)
            myWrench.force.z = 0;         
          myWrench.force.z=temp*(pow(16,3-(num-1)%4))+ myWrench.force.z;
          }
      else if(num/4 == 3)//fourth data
	  {
          if(num%4 == 1)
            myWrench.torque.x = 0;
          myWrench.torque.x=temp*(pow(16,3-(num-1)%4))+ myWrench.torque.x;
          }
      else if(num/4 == 4)//fifth data
	  {
          if(num%4 == 1)
            myWrench.torque.y = 0;
          myWrench.torque.y=temp*(pow(16,3-(num-1)%4))+ myWrench.torque.y;
          }
      else if(num/4 == 5)//sixth data
          {
          if(num%4 == 1)
            myWrench.torque.z = 0;
          myWrench.torque.z=temp*(pow(16,3-(num-1)%4))+ myWrench.torque.z;

	  
          }
    }
   }   
   } 
    
  printf("%s\n",buffer);
  printf("%d %d %d\n ",myWrench.force.x,myWrench.force.y,myWrench.force.z);
  myWrench_final.force.x = (myWrench.force.x-8193)/sensitive_Fx;
  myWrench_final.force.y = (myWrench.force.y-8193)/sensitive_Fy;
  myWrench_final.force.z = (myWrench.force.z-8193)/sensitive_Fz;
  myWrench_final.torque.x = (myWrench.torque.x-8193)/sensitive_Mx;
  myWrench_final.torque.y = (myWrench.torque.y-8193)/sensitive_My;
  myWrench_final.torque.z = (myWrench.torque.z-8193)/sensitive_Mz;
  //publish the mystring,myWrench,myWrench_final*/
    my_publisher_object.publish(mystring);
    printf("%s",buffer);
    // my_publisher_object2.publish(myWrench);
    // my_publisher_object3.publish(myWrench_final);

      loopRate.sleep();			
     // The call to sleep makes this node's while loop to be called every 1Hz.
    }




     close(sockfd); //close the socket
    return 0;


}
