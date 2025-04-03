#include <Adafruit_VL53L0X.h>
#include "StepPin.h"


// run đích ok , đang lỗi thuật toán 


//#include <Arduino.h>
#include <iostream>
#include <stack>
#include <vector>
#include <queue>
#include <set>
#include <cstring>
#include <limits>
#include <iostream>
#include <stack>
#include <vector>
#include <queue>
#include <set>
#include <cstring>
#include <limits>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <stack>



using namespace std;
int Front,Left, Right;
int target = 0;

// ==================================================================== Mussic ====================================================================
int melody[] = {
  262, 262, 392, 392, 440, 440, 392,
  349, 349, 330, 330, 294, 294, 262,
  392, 392, 349, 349, 330, 330, 294,
  392, 392, 349, 349, 330, 330, 294,
  262, 262, 392, 392, 440, 440, 392,
  349, 349, 330, 330, 294, 294, 262
};
int noteDuration[] = {
  4, 4, 4, 4, 4, 4, 2,
  4, 4, 4, 4, 4, 4, 2,
  4, 4, 4, 4, 4, 4, 2,
  4, 4, 4, 4, 4, 4, 2,
  4, 4, 4, 4, 4, 4, 2,
  4, 4, 4, 4, 4, 4, 2
};

// ==================================================================== End of music ====================================================================

// ==================================================================== MAZE SOLVER ====================================================================
#define UP 0
#define	DOWN 1
#define	LEFT 2
#define	RIGHT 3
const  int rows=16;
const  int cols=16;

typedef struct coor{
    int row;
    int col;
    int value;
}coord;
typedef struct cell_infos{
	// variables for north,east,south,west walls
	bool walls[4];
	bool visited;
    int angle_update;
    bool dead=0;
}cell_info;
typedef struct wall_mazes{
	cell_info cells[16][16];
}wall_maze;
void log(const std::string& text) {
    std::cerr << text << std::endl;
}
bool isValid(int x, int y) {
    return (x >= 0 && x < rows && y >= 0 && y < cols);
}


wall_maze maze;
const int dx[] = {1, -1, 0, 0};
const int dy[] = {0, 0, -1, 1};
std::queue<coord> myQueue; // 
void init_arr(std::vector<std::vector<int>> &arr,int row,int col)
{
    for(int i = 0 ;i<row;i++)
    {
        std::vector<int> arr_row;
        for(int j = 0 ;j<col;j++)
        {
            arr_row.push_back(-1);
        }
        arr.push_back(arr_row);
    }
}
void check_and_fill(std::vector<std::vector<int>> &arr,int row,int col,int value)
{
    if(row<0 ||col<0||row>=arr.size()||col>=arr[0].size()||arr[row][col]!=-1)return;
    value+=1;
    coord point={row,col,value};
    myQueue.push(point);
    arr[row][col]=value;
}
void init_flood(std::vector<std::vector<int>> &arr,int row,int col)
{
   
    int count_=0;
    coord point={row,col,count_};
    myQueue.push(point);
    arr[row][col]=0;
    coord point2={row+1,col,count_};
    myQueue.push(point2);
    arr[row+1][col]=0;
    coord point3={row,col+1,count_};
    myQueue.push(point3);
    arr[row][col+1]=0;
    coord point4={row+1,col+1,count_};
    myQueue.push(point4);
    arr[row+1][col+1]=0;
while (!myQueue.empty()) {
        coord frontCoord = myQueue.front(); 
        myQueue.pop(); 
        check_and_fill(arr,frontCoord.row+1,frontCoord.col,frontCoord.value);
        check_and_fill(arr,frontCoord.row-1,frontCoord.col,frontCoord.value);
        check_and_fill(arr,frontCoord.row,frontCoord.col+1,frontCoord.value);
        check_and_fill(arr,frontCoord.row,frontCoord.col-1,frontCoord.value);
    }
    
    
}
void init_flood_start(std::vector<std::vector<int>> &arr,int row_,int col_,int back_=0)
{
    
    int count_=0;
    for(int i=0;i<16;i++)
    {
        for(int j = 0 ;j<16;j++)
        {
            arr[i][j]=-1;
            if(back_==2&&maze.cells[i][j].visited==false){
                arr[i][j]=255;
                maze.cells[i][j].dead=true;
            }
           
        }
        //  if(back_==2)std::cerr<<""<<std::endl;
    }
    if(back_!=1)
    {
        coord point2={row_+1,col_,count_};
        myQueue.push(point2);
        arr[row_+1][col_]=0;
        coord point3={row_,col_+1,count_};
        myQueue.push(point3);
        arr[row_][col_+1]=0;
        coord point4={row_+1,col_+1,count_};
        myQueue.push(point4);
        arr[row_+1][col_+1]=0;
    }
    coord point={row_,col_,count_};
    myQueue.push(point);
    arr[row_][col_]=0;
    while(!myQueue.empty())
    {
        coord frontCoord = myQueue.front(); 
        myQueue.pop(); 
          for (int i = 0; i < 4; ++i) {
                int newRow = frontCoord.row + dy[i]; // 0 0 -1 1
                int newCol = frontCoord.col + dx[i]; //1 -1 0 0 
                // std::cerr <<dir<< new_cell.walls[dir]<<std::endl ;
                // bool check_=check_wall_angle(maze.cells[cur_stack.row][cur_stack.col],ind_);
                bool check_=maze.cells[frontCoord.row][frontCoord.col].walls[i];
                if(!check_)check_and_fill(arr,newRow,newCol,frontCoord.value);
          }
        //   std::cerr<<"size:"<<myQueue.size()<<std::endl;
          if(myQueue.size()>120){
            log("fulllll");
            break;
          }
    } 
}
void update_wall_debug(std::vector<std::vector<int>> &arr)
{
    char dir;
    bool clear_=0;
    int count_=0;
      for(int i= 0;i<16;i++)
    {
        for(int j = 0;j<16;j++)
        {
            std::string value=std::to_string(arr[i][j]);
            for(int k = 0;k<4;k++)
            {
                clear_=maze.cells[i][j].walls[k];
              
                if(k==0)dir='n';
                else if(k==1)dir='s';
                else if(k==2)dir='w';
                else dir='e';
                if(clear_);//Wall(i,j,dir);               
            }
            if(maze.cells[i][j].visited==true)
            {
                // log("blue");
                //Color(i,j,'b'); //g-green r-red b-Blue
                
            }
//            else API::clearColor(i,j); //g-green r-red b-Blue
            if(maze.cells[i][j].dead==true)
            {
                //Text(i,j,"Dead"); //g-green r-red b-Blue
                //Color(i,j,'r'); //g-green r-red b-Blue
           
            }
            // value+="-R:"+std::to_string(i)+"-C:"+std::to_string(j);
             //Text(i,j,value);
        }
    }
}
bool check_wall_angle(cell_info cell,int &dir)
{
    switch(cell.angle_update)
    {
        case 90:
            break;
        case 270:
            if(dir%2==0)dir+=1;
            else dir-=1;
            break;
        case 0:
            if(dir==0 || dir ==1)dir+=2;
            else if(dir==2)dir=1;
            else dir=0;
            break;
        case 180:
             if(dir==2 || dir ==3)dir-=2;
            else if(dir==0)dir=3;
            else dir=2;
            break;
    }
    return cell.walls[dir];
}
cell_info cell_direction_adjust(cell_info cell)
{
    //  std::cerr<<"A:"<<cell.angle_update<<"-L:";
    cell_info cell_new;
    cell_new=cell;
    for(int i=0;i<4;i++)
    {
        int ind = i;
    // std::cerr<<cell.walls[i];
    // std::cerr<<i<<"->";
    switch(cell.angle_update)
        {
            case 90:
                break;
            case 270:
                if(i%2==0)ind+=1;
                else ind-=1;
                break;
            case 0:
                if(i==0 || i ==1)ind+=2;
                else if(i==2)ind=1;
                else ind=0;
                break;
            case 180:
                if(i==2 || i ==3)ind-=2;
                else if(i==0)ind=3;
                else ind=2;
                break;
        }
        //  std::cerr<<ind<<"|";
        cell_new.walls[i]=cell.walls[ind]; 
    }  
    return cell_new;
}
void go_to_cell(int &angle_now,int dir)
{
    switch(dir)
            {
                case -1:
                    log("not dir");
                    break;
                case UP:
                    // log("forward");
                    go_mm(175);
                    ReadTof();
                    if (Left<100)
                    FixError(Left);
                    // delay(300);
                    break;
                case DOWN:
                    // log("Down");
                    angle_now-=180;
                     TurnRight();
                     TurnRight();
                    back_mm(70);
                    go_mm(40);
                     go_mm(175);
                     ReadTof();
                     if ( Left < 110)
                       target = Left;
                      break;
                case LEFT:
                    // log("Left");
                    angle_now+=90;

                    TurnLeft();
                    if ( Right < 110){
                      back_mm(70);
                      go_mm(40);                      
                    }

                    go_mm(175);
                    ReadTof();
                    if (Left < 110)
                    target = Left;
                    break;
                case RIGHT:
                    // log("right");
                    angle_now-=90;
                    TurnRight();
                    if ( Left < 110){
                      back_mm(70);
                      go_mm(48);                      
                    }
                    go_mm(175);
                    ReadTof();
                    if ( Left < 110)
                      target = Left;
                    break;
                default:
                    break;
            }
            angle_now = angle_now % 360;
            // ï¿½?m b?o gï¿½c khï¿½ng b? ï¿½m
            if (angle_now < 0) {
                angle_now += 360;
            }
}
coord get_min_neighbour(cell_info cell_wall,coord cur,std::vector<std::vector<int>> &arr,bool change_=0)
{
    int min_neightbor=255;
    coord next_step;
    next_step.value=-1;
    int ind;
     for (int dir = 0; dir < 4; ++dir) {
                int newRow = cur.row + dy[dir]; // 0 0 -1 1
                int newCol = cur.col + dx[dir]; //1 -1 0 0 
                ind=dir;
                bool check_=cell_wall.walls[dir];
                if(change_)check_=check_wall_angle(cell_wall,ind);
                // std::cerr << check_;
                if(isValid(newRow,newCol) && !check_)
                { 
                    if(arr[newRow][newCol]<=min_neightbor)
                    { 
                        min_neightbor=arr[newRow][newCol];
                        next_step.row=newRow;
                        next_step.col=newCol;
                        next_step.value=ind;
                    }
                }
            }
    return next_step;
}
void flood(std::stack<coord>& stack_flood,std::vector<std::vector<int>> &arr)
{
    coord cur_stack;
    coord next_step;
     while(!stack_flood.empty())
        {
            cur_stack=stack_flood.top();
            stack_flood.pop(); 
            int min_neightbor=255;
            bool check_;
            next_step=get_min_neighbour(maze.cells[cur_stack.row][cur_stack.col],cur_stack,arr);
            min_neightbor=arr[next_step.row][next_step.col];
            if(arr[cur_stack.row][cur_stack.col]-1 != min_neightbor )
            {
                for(int i =0 ;i<4;i++)
                {
                    coord cur_add;
                    cur_add.row= cur_stack.row + dy[i]; // 0 0 -1 1
                    cur_add.col= cur_stack.col + dx[i]; //1 -1 0 0 
                    check_=maze.cells[cur_stack.row][cur_stack.col].walls[i];
                    if(isValid(cur_add.row,cur_add.col) &&arr[cur_add.row][cur_add.col]!=0&&!check_)
                    {
                        stack_flood.push(cur_add);
                    }
                }
                if(arr[cur_stack.row][cur_stack.col]!=0)arr[cur_stack.row][cur_stack.col]=min_neightbor+1;
                // update_wall_debug(arr);
                // log("added");
            }
            int stack_size=stack_flood.size();
            if(stack_size>=35){
                log("full stack");
                for(int i=0;i<stack_size;i++)
                {
                    stack_flood.pop();
                }
                return;
            }
        }
}
cell_info update_walls(int angle_now,int row,int col)
{
    cell_info new_cell;
    ReadTof();
    if ( Front < 110)
      KeepDistance(50);
      delay(200);
    new_cell.angle_update=angle_now;
    new_cell.walls[UP]=(Front < 110);
    new_cell.walls[DOWN]=0;
    new_cell.walls[LEFT]=(Left < 110);
    new_cell.walls[RIGHT]=(Right < 110);
    new_cell.dead=0;
    new_cell.visited=1;

    maze.cells[row][col]=cell_direction_adjust(new_cell);
    if(new_cell.walls[UP]==1&&new_cell.walls[LEFT]==1&&new_cell.walls[RIGHT]==1&&row!=0&&col!=0)
    {
        log("dead");
         maze.cells[row][col].dead=1;
    }
    for(int i=0;i<4;i++)
    {
        int newRow=row+dy[i];
        int newCol=col+dx[i];
        if(isValid(newRow,newCol))
        {
            if(i==UP)maze.cells[newRow][newCol].walls[DOWN]=maze.cells[row][col].walls[UP];
            else if(i==LEFT)maze.cells[newRow][newCol].walls[RIGHT]=maze.cells[row][col].walls[LEFT];
            else if(i==RIGHT)maze.cells[newRow][newCol].walls[LEFT]=maze.cells[row][col].walls[RIGHT];
        }
    }
    return new_cell;
}
coord floodfill(coord start,coord dest,std::vector<std::vector<int>> &arr,int &angle_now)
{
    std::queue<coord>path_queue;
    path_queue.push(start);
    coord cur=start;
    cell_info new_cell;
    // cell_info new_cell;
    std::stack<coord>stack_flood;
    stack_flood.push(start);
    int path_distance_value_find=0;
    int save_row,save_col;
    coord next_step;
    while(1)
    {
        if(!path_queue.empty()) // dua ra quyet dinh va go
        {
            cur=path_queue.front();
            new_cell=update_walls(angle_now,cur.row,cur.col);
            if(arr[cur.row][cur.col]==arr[dest.row][dest.col])break;
            flood(stack_flood,arr);
            path_queue.pop(); 
            next_step=get_min_neighbour(new_cell,cur,arr,1);
            path_queue.push(next_step);
            stack_flood.push(next_step);
            go_to_cell(angle_now,next_step.value);    
            path_distance_value_find++; 
        }
        else{
            log("empty Queue- break");
            break;
        }
        // std::cerr<<"cur:"<<cur.value<<"-dest:"<<dest.value<<std::endl;
    }
    while(!path_queue.empty()) path_queue.pop(); 
    // new_cell=update_walls(angle_now,cur.row,cur.col);
    
    std::cerr<<"total_cost:"<<path_distance_value_find<<std::endl;
    coord p_return={next_step.row,next_step.col,0};
    return p_return;
}          
void init_maze()
{
    for(int i =0;i<16;i++)
    {
        for(int j=0;j<16;j++)
        {
            maze.cells[i][j].visited=0;
            maze.cells[i][j].angle_update=90;
            maze.cells[i][j].dead=0;
            for(int k = 0 ;k<4;k++)maze.cells[i][j].walls[k]=0;
        }
    }
}
void go_to_cell_shorted(int &angle,int dir)
{
    int new_dir=dir;
        switch(angle)
        {
            case 90:
                break;
            case 270:
                if(dir%2==0)new_dir+=1;
                else new_dir-=1;
                break;
            case 0:
                if(dir==0 || dir ==1)new_dir+=2;
                else if(dir==2)new_dir=1;
                else new_dir=0;
                break;
            case 180:
                if(dir==2 || dir ==3)new_dir-=2;
                else if(dir==0)new_dir=3;
                else new_dir=2;
                break;
        }
    go_to_cell(angle,new_dir);
}
char path[255];
char step = 0; 
void shorted_path_go(std::vector<std::vector<int>> &arr,int angle_now,coord start,coord dest)
{
   
    std::queue<int>next_dir_path;
    cell_info new_cell;
    // cell_info new_cell;
    int save_row,save_col;
    coord cur=start;
    int angle=angle_now;
    for(int i=0;i<arr[start.row][start.col];i++)
    {
            int next_dir=-1;
            int newRow;
            int newCol;
            for (int dir = 0; dir < 4; ++dir) {
                newRow = cur.row + dy[dir]; // 0 0 -1 1
                newCol = cur.col + dx[dir]; //1 -1 0 0 
                bool check_=maze.cells[cur.row][cur.col].walls[dir];
                if(isValid(newRow,newCol) && !check_)
                {
                    if(arr[newRow][newCol]<arr[cur.row][cur.col])
                    { 
                        next_dir=dir;
                        save_row=newRow;
                        save_col=newCol;
                    }
                }
            }
            if(next_dir!=-1)
            {
                cur.row=save_row;
                cur.col=save_col;
                next_dir_path.push(next_dir);
                //Color(save_row,save_col,'w');
                //Text(save_row,save_col,std::to_string(next_dir));  // to_string(arr[save_row][save_col])
            }
            path[step++] = next_dir;
    }
    for (int i = 0 ; i < step ; i ++)
    	 std::cerr<<(int)path[i]<<std::endl;
}

  void sound ( int max , int min ){
    int x=  min ;
    bool check  = true ; 
    while(true){
      if(check == true ){
        tone(14,x,300);
        delay(50);
        x+=20;
      if(x == max  ) check = false; 
    }
    else {
      tone(14,x,300);
      delay(50);
      x-=20;
      if( x == min) check = true;
    }
    }
  }
 int MazeSolve()
{
	std::vector<std::vector<int>>arr;
    init_arr(arr,rows,cols);
    init_flood(arr,7,7);
    init_maze();
    // test_maze()
    coord start={0,0,arr[0][0]};
    coord dest={7,7,arr[7][7]};
    //Color(0,0,'r');
    //Color(7,7,'r');
    //Text(0,0,"Start");
    //Text(7,7,"Goal");
    update_wall_debug(arr);
    int angle_now=90;
    coord new_coord;


    new_coord=floodfill(start,dest,arr,angle_now);
    init_flood_start(arr,0,0,1);
//     update_wall_debug(arr);
    std::cerr<<"done2"<<std::endl;
    for (int i = 0; i < sizeof(melody) / sizeof(int); i++) {
    tone(14, melody[i], 1000/noteDuration[i]);
    delay(1000/noteDuration[i]);
    noTone(14);
  }
    new_coord=floodfill(new_coord,start,arr,angle_now);
    init_flood_start(arr,7,7,2);
//     update_wall_debug(arr);
   
   // shorted_path_go(arr,angle_now,new_coord,dest);
	}

// ============================================================================= TOF SETUP  =======================================
// address we will assign for all 4 sensor
#define LOX1_ADDRESS 0x30
#define LOX2_ADDRESS 0x31
#define LOX3_ADDRESS 0x32


// set the pins to shutdown for all 4 sensors
#define SHT_LOX1 2  // right 
#define SHT_LOX2 19 // front
#define SHT_LOX3 25 //left


int corn_count = 0;
int pid_done = 1;
// objects for the vl53l0x
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();

// this holds the measurement
VL53L0X_RangingMeasurementData_t measure1;
VL53L0X_RangingMeasurementData_t measure2;
VL53L0X_RangingMeasurementData_t measure3;

// ============================================================================ TOF FUNCTION ========================================

void setID() {
  // all reset
  digitalWrite(SHT_LOX1, LOW);    
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(SHT_LOX3, LOW);    

  delay(10);
  // all unreset
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, HIGH);
  digitalWrite(SHT_LOX3, HIGH);
 
  delay(10);

  // activating LOX1 and reseting LOX2
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(SHT_LOX3, LOW);


  // initing LOX1
  if(!lox1.begin(LOX1_ADDRESS)) {
    Serial.println(F("Failed to boot first VL53L0X"));
    while(1);
  }
  delay(10);

  ///************************* sensor 2 activation 
  // activating LOX2
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);

  //initing LOX2
  if(!lox2.begin(LOX2_ADDRESS)) {
    Serial.println(F("Failed to boot second VL53L0X"));
    while(1);
  }
  
   ///************************* Right activation  
  // activating LOX3
  digitalWrite(SHT_LOX3, HIGH);
  delay(10);

  //initing LOX3
  if(!lox3.begin(LOX3_ADDRESS)) {
    Serial.println(F("Failed to boot third VL53L0X"));
//    while(1);
  }  
  
}

void ReadTof() {
  
  lox1.rangingTest(&measure1, false); // pass in 'true' to get debug data printout!
  lox2.rangingTest(&measure2, false); // pass in 'true' to get debug data printout!
   lox3.rangingTest(&measure3, false); // pass in 'true' to get debug data printout!

  
  // print sensor one reading
  if(measure1.RangeStatus != 4) {     // if not out of range
    Front = measure1.RangeMilliMeter;    


    
    Serial.print(Front);
    Serial.print("   ");
  } 

  // print sensor two reading
  if(measure2.RangeStatus != 4) {
    Left = measure2.RangeMilliMeter;
    Serial.print(Left);    Serial.print("   ");
  } 

   ///Robojax.com code see video https://youtu.be/0glBk917HPg
  // // print sensor three reading
  if(measure3.RangeStatus != 4) {
    Right = measure3.RangeMilliMeter;
    Serial.println(Right);
  } 
  

}
void SetupTof(){
  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);
  pinMode(SHT_LOX3, OUTPUT);

  Serial.println("Shutdown pins inited...");

  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(SHT_LOX3, LOW);

  Serial.println("All four in reset mode...(pins are low)");
  
  Serial.println("Starting...");
  tone(14,300,100);
  setID();
  tone(14,1000,100);
  ReadTof();

}


// ============================================================================ Stepper setup ==========================================================================
float speed = 500;
int turn_step = 122;
// D  = 67mm
// 1 step = 1.8 degree = 200 step
// 1 step == 1.0048

void SetPinStep(){
  
    pinMode(LeftDir,OUTPUT);
    pinMode(LeftStep,OUTPUT);
    pinMode(RightDir,OUTPUT);
    pinMode(RightStep,OUTPUT);

}
bool check = 1;
//    Go straight mm for both wheels
void go_mm( int mm){                       
  // int step = mm/1.0048*16;
 int step = mm/1.00*16; 
  speed = 1000;
  // set direction forward
  digitalWrite(LeftDir,LeftFord);
  digitalWrite(RightDir,RightFord);
  for (int i = 0; i < step*3/4 ; i ++){ 
    digitalWrite(RightStep,HIGH);
    digitalWrite(LeftStep,HIGH); 
 
    digitalWrite(LeftStep,LOW); 
    digitalWrite(RightStep,LOW); 
    delayMicroseconds(speed); 
      if ( speed > 500)
      speed -= 1 ;
    // else
    //   speed+= 5;
  }
    check = !check;
    for (int i = 0; i < step/4 ; i ++){ 
    digitalWrite(RightStep,HIGH);
    digitalWrite(LeftStep,HIGH); 
 
    digitalWrite(LeftStep,LOW); 
    digitalWrite(RightStep,LOW); 
    delayMicroseconds(speed); 
    speed += 0.1;
  }
}
void back_mm(int mm){
    int step = mm/1.0048*16;

  // set direction backward
  digitalWrite(LeftDir,LeftBack);
  digitalWrite(RightDir,RightBack);
  for (int i = 0; i < step ; i ++){ 
    digitalWrite(RightStep,HIGH);
    digitalWrite(LeftStep,HIGH); 
 
    digitalWrite(LeftStep,LOW); 
    digitalWrite(RightStep,LOW); 

    delayMicroseconds(500); 
  }

}
void TurnRight(){
  digitalWrite(LeftDir,LeftFord);
  digitalWrite(RightDir,RightBack);
  for (int i = 0 ;  i < turn_step*8; i++){
    digitalWrite(LeftStep,HIGH);
    digitalWrite(LeftStep,LOW); 
    digitalWrite(RightStep,HIGH);
    digitalWrite(RightStep,LOW); 
    delayMicroseconds(1000);     
  }
}

void TurnLeft(){
    digitalWrite(LeftDir,LeftBack);
  digitalWrite(RightDir,RightFord);
  for (int i = 0 ;  i < turn_step*8; i++){

    digitalWrite(LeftStep,HIGH);
    digitalWrite(LeftStep,LOW); 
    digitalWrite(RightStep,HIGH);
    digitalWrite(RightStep,LOW); 
    delayMicroseconds(1000);     
  }
}

void KeepDistance(int target){
  int step = Front - target;
  step = step/1.0048*16;
  if ( Front > target){
    digitalWrite(LeftDir,LeftFord);
    digitalWrite(RightDir,RightFord);
  }
  else
  {
    digitalWrite(LeftDir,LeftBack);
    digitalWrite(RightDir,RightBack);
  }
  for (int i = 0; i < step ; i ++){ 
    digitalWrite(RightStep,HIGH);
    digitalWrite(LeftStep,HIGH); 
 
    digitalWrite(LeftStep,LOW); 
    digitalWrite(RightStep,LOW); 
    delayMicroseconds(1000); 
    // if ( speed > 200)
    //   speed -= 3 ;
  }
}

void FixError(int value){
    int step = (value - target)/1.0048*11;
    int speed2 = 500;
  // set direction forward
  if ( step > 0 ){
    digitalWrite(LeftDir,LeftBack);
  }
  else {
    step = -step;
    digitalWrite(LeftDir,LeftFord);
  }

  Serial.println(step);
  for (int i = 0; i < step ; i ++){ 
    // digitalWrite(RightStep,HIGH);
    digitalWrite(LeftStep,HIGH); 
    digitalWrite(LeftStep,LOW); 
    // digitalWrite(RightStep,LOW); 
    delayMicroseconds(speed2); 
    // if ( speed > 200)
    //   speed -= 3 ;
  }
}
// ========================================================================== End Step setup ==========================================================================

// ========================================================================== Main function ==========================================================================

int pin [4] = {35,34,39,36};

int mode = 0;


void setup() {
  SetPinStep();
  for (int i = 0 ; i < 4 ; i ++)
    pinMode(pin[i],INPUT);
  pinMode(14,OUTPUT);
  mode = digitalRead(pin[0]);
  Serial.begin(115200);


  if ( mode){
  tone(14,2000,300);
  }
   SetupTof();
  while ( touchRead(T4) > 70){}
  tone(14,1000,100);
   ReadTof();
  target = Left;
}
//===========================================define running maze=================================

// ==================================================================== END OF MAZE SOLVER ====================================================================  

void canchinh(){
  back_mm(60);
  go_mm(40);
  delay(100);
}
void loop() { 



  go_mm(40);
  MazeSolve();
  
  TurnRight();
  TurnRight();
  back_mm(60);

  // go_mm(180);
  // go_mm(180);
  // go_mm(180);

//  ReadTof();
}




  
