#include<bits/stdc++.h>
#include<windows.h>
#include<conio.h>
using std::cout;
using std::cin;
using std::min;
using std::max;
using std::string;
using std::to_string;
using std::ostream;
using std::vector;
using std::map;
using std::stringstream;

//系统常数定义
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

//全局函数区 
void setCursorPosition(int x=0,int y=0){//设置系统光标位置
	COORD coord;
	coord.X=x,coord.Y=y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),coord);
}

void color_print(int char_color=7,int background_color=0){//设置输出颜色
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),char_color&0xF|(background_color&0xF)<<4);
}

void hidden(bool pd=true){//设置系统光标显示状态
	CONSOLE_CURSOR_INFO cci;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&cci);
	cci.bVisible=!pd;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&cci);
}

char game_getch(){//制作游戏时使用的getch
	unsigned char t=getch();
	if((t==0||t==224)&&(t=getch())){
		switch(t){
			case 72:return'w';
			case 80:return's';
			case 75:return'a';
			case 77:return'd';
		}
		return'\0';
	}
	return(t>='A'&&t<='Z')*' '+t;
}

long long random(long long a,long long b){//随机数
	return double(rand())/(RAND_MAX+1)*(max(a,b)-min(a,b)+1)+min(a,b);
}

template<typename T=unsigned long long>
const T timems(){//C98的毫秒函数
	SYSTEMTIME c;
	volatile T t=time(0);
	GetSystemTime(&c);
	return(t/60*60+c.wSecond+(t%60>c.wSecond)*60)*1000+c.wMilliseconds;
}

void srs(){//初始化随机数种子
	srand(timems());
}

template<typename T>
constexpr const T& clamp(const T& x,const T& mi,const T& ma){//收斂函數
	return x<mi?mi:(x>ma?ma:x);
}

template<typename T>
string operator *(const string& str,const T& a){//快速幂字符串
	string s=str,jg="";
	for(T num=a,idx=1;num;idx<<=1,num>>=1,s+=s)if(num&1)jg+=s;
	return jg;
}

template<typename T>
ostream& operator<<(ostream& os,const vector<T>& v){//vector输出重载;
	os<<"[";
	for(int i=0;i<v.size();os<<v[i++])if(i)os<<",";
	os<<"]";
	return os;
}

template<typename T1,typename T2>
ostream& operator<<(ostream& os,const map<T1,T2>& mp){//map输出重载
	bool pd=false;
	os<<"{";
	for(typename map<T1,T2>::const_iterator it=mp.begin();it!=mp.end();os<<it->first<<":"<<it->second,++it,pd=true)if(pd)os<<",";
	os<<"}";
	return os;
}

int color_term_convert(int color){
	color&=0xF;
	return(color>>2&0b1)|(color&0b1010)|(color&0b1)<<2;
}

string color_str(int char_color=7,int back_color=0){//返回颜色的终端转义字符串
	char_color&=0xF,back_color&=0xF;
	return"\033["+to_string(3+6*(char_color>>3))+to_string(color_term_convert(char_color&0x7))+";"+to_string(4+6*(back_color>>3))+to_string(color_term_convert(back_color&0x7))+"m";
}

/*
## 关于命名
FurryConsole 的“Furry”是对以下技术的隐喻：
- 局部更新（像兽梳毛般精准）
- 流式语法（如尾巴摆动般自然）
- 视觉温暖（抵抗数字世界的冰冷）

> 或许有一天，它会真的长出爪子
*/

/*
底层习惯：
如果不是面向与外部交互，统一按照[x行][y列]的形式构建坐标系
否则，参数名保证是建立在传统坐标系上xy，但内部仍然需要按照[x行][y列]的方式处理，
*/

//核心类 
class FurryConsole{
public://公共
	//初始化区 
	void init(){//初始化函数
		srs();
		hidden();
		cursorPointer={0,0,7,0};
		cls(false);
		enableAnsiSupport();
	}

	void enableAnsiSupport(){
		HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);
		if(hOut!=INVALID_HANDLE_VALUE){
			DWORD mode;
			GetConsoleMode(hOut,&mode);
			SetConsoleMode(hOut,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}
	}
	
	FurryConsole(int reserveN=0,int reserveM=0){//创建初始化
		init();
		tempScreen=screen=vector<vector<pxChar>>(reserveN,vector<pxChar>(reserveM,{' ',0,0}));
	} 
	
	//量区
	//变量区
	int tab_width=8;
	
	//自定义类型区
	//底层类型区
	struct pxChar{//字符像素
		char c;
		int charColor,backColor;
	};
	vector<vector<pxChar>>tempScreen,screen;
	
	struct cursor{//模拟虚拟光标指针
		int x,y,charColor,backColor;
	}cursorPointer;

	struct pos{//位置
		int x,y;
	};

	class region{//区域（用于维护变化的区域）
	public:
		void push(const pos& p){//针对添加的坐标进行扩容
			if(pd)max_x=max(max_x,p.x),min_x=min(min_x,p.x),max_y=max(max_y,p.y),min_y=min(min_y,p.y);
			else min_x=max_x=p.x,max_y=min_y=p.y;
			pd=true;
		}
		void clean(){min_x=max_x=max_y=min_y=~(pd=false);}
		int maxX()const{return max_x;}
		int minX()const{return min_x;}
		int maxY()const{return max_y;}
		int minY()const{return min_y;}
		bool enable()const{return pd;}
	private://私用变量，不然会被外部修改导致错误
		bool pd=false;//启用状态
		int max_x=-1,min_x=-1,max_y=-1,min_y=-1;//边缘位置
	}changeZone;

	//API指令区
	struct colorAtionCmd{//着色指令
		int charColor,backColor;
	};

	struct setPositionCmd{//设置光标位置指令
		int x,y;
	};
	
	//关键函数：类的实际作用与外部交互的函数
	void testCout(){//调试信息函数
		system("cls");
		color_print();
		for(int i=0;i<screen.size();++i){
			for(int j=0;j<screen[i].size();++j)cout<<"c<"<<screen[i][j].c<<">"<<"co:"<<screen[i][j].charColor<<"|"<<screen[i][j].backColor<<" ";
			cout<<"\n";
		}
	}
	
	//API函数（可单独调用，也可以作为内联输出流的调用接口）
	void colorAtion(int cc=7,int bc=0){//提供给外部的变色函数
		cursorPointer.charColor=cc,cursorPointer.backColor=bc;
	}
	
	void setPosition(int x=0,int y=0){//提供给外部设置光标位置函数  
		cursorPointer.x=y,cursorPointer.y=x;
	}
	
	void cls(bool pd=true){//清空渲染，false为不调用system，true则调用
		if(pd)system("cls");
		cursorPointer.x=0,cursorPointer.y=0;
		for(int i=0;i<screen.size();++i)for(int j=0;j<screen[i].size();++j)screen[i][j]={' ',7,0};
		tempScreen=screen;
	}

	void calibrateTabWidth(){//校准Tad键的占位字数
		cls();
		int x=0;
		char c;
		cout<<"请校准Tad键的占位字数，以配置FurryConsole类有关\'\\t\'的操作进行正确处理\n";
		cout<<"请让箭头“V”与箭头“A”指向同一块位置，并按下回车键\n";
		cout<<"\tV";
		cout<<"\nA";
		while(1){
			c=game_getch();
			if(c=='\r')break;
			setArrayPosition(3,x);
			cout<<" ";
			x+=(c=='d')-(c=='a');
			x=clamp(x,0,16);
			setArrayPosition(3,x);
			cout<<"A";
		}
		tab_width=x;
		cls();
	}

	//输出流交互函数，像cout一样使用FurryConsole！
	template<typename T>
	FurryConsole& operator<<(const T& value){//一般类型的输出
		stringstream ss;
		ss<<value;
		outstring(ss.str());
		return *this;//返回自身，支持链式调用
	}
	
	FurryConsole& operator<<(const colorAtionCmd& value){//着色命令
		colorAtion(value.charColor,value.backColor);
		return *this;
	}
	
	FurryConsole& operator<<(const setPositionCmd& value){//设置光标命令
		setPosition(value.x,value.y);
		return *this;
	}
	
	//渲染器函数（动态数组版）
	//暴力渲染器函数
	void xr(){//老一套的无脑渲染器，多用作跑分比较
		if(!changeZone.enable())return;
		setArrayPosition();
		for(int i=changeZone.minX();i<=changeZone.maxX();++i){
			for(int j=0;j<=min((long long)screen[i].size()-1,(long long)changeZone.maxY());++j)color_print(screen[i][j].charColor,screen[i][j].backColor),cout<<screen[i][j].c;
			cout<<"\n";
		}
		tempScreen=screen;
		changeZone.clean();
	}

	//API调用优化渲染器函数
	void xrPro(){//正在维护的渲染器（面向全用户的兼容性渲染优化函数）
		if(!changeZone.enable())return;
		cursor xrzz={-1,-1,-1,-1};//光标指针
		for(pos p=firstpos();posLegal(p);){//枚举每一个位置
			if(pxdif(tempScreen[p.x][p.y],screen[p.x][p.y])){//如果与旧像素不相似
				string out="",undout="";//out:输出字符串 undout:预备字符串
				pxChar special=screen[p.x][p.y];//代表性像素
				pos temppos=p;//记录起始位置
				int x=p.x,y=p.y;//手动计算渲染后的位置
				while(posLegal(p)&&pxsim(special,screen[p.x][p.y])){//合法且与代表性像素相似
					if(pxdif(tempScreen[p.x][p.y],screen[p.x][p.y]))out+=undout+pushPxChar(screen[p.x][p.y]),y+=undout.size()+1,undout="";//如果与旧像素不同，连接预备字符串和当前字符到输出
					else undout+=pushPxChar(screen[p.x][p.y]);//连接预备字符串
					tempScreen[p.x][p.y]=screen[p.x][p.y],special=pxspecial(special,screen[p.x][p.y]);//更新信息
					int line=addpos(p);//累加，同时计算跳过的行数
					if(line>0)out+=string("\n")*line,x+=line,y=0,undout="";//有跳过的行，直接连接
				}
				if(xrzz.x!=temppos.x||xrzz.y!=temppos.y)setArrayPosition(temppos.x,temppos.y);//移到指定位置（小优化）
				if(xrzz.charColor!=special.charColor||xrzz.backColor!=special.backColor)color_print(special.charColor,special.backColor);//设置颜色（小优化）
				cout<<out,xrzz={x,y,special.charColor,special.backColor};//输出，同时光标指针更新
			}
			else addpos(p);//迭代
		}
		changeZone.clean();
	}

	//终端渲染器函数（极简且效率最快，但要注意兼容性）
	void xrPlus(){
		if(!changeZone.enable())return;
		setArrayPosition();
		string out="";
		pxChar temp={'\0',7,0};
		for(int i=changeZone.minX();i<=changeZone.maxX();++i){
			for(int j=0;j<=min((long long)screen[i].size()-1,(long long)changeZone.maxY());++j){
				if(temp.charColor!=screen[i][j].charColor||temp.backColor!=screen[i][j].backColor)out+=color_str(screen[i][j].charColor,screen[i][j].backColor),temp=screen[i][j];
				out+=pushPxChar(screen[i][j]);
			}
			out+="\n";
		}
		out+=color_str();
		cout<<out;
		changeZone.clean();
	}

private://内部使用
	pos firstpos(){
		for(int i=changeZone.minX();i<=changeZone.maxX();++i)if(screen[i].size())return{i,0};
		return{-1,-1};
	}

	int addpos(pos& p){
		for(int i=p.x,x=p.x;i<=changeZone.maxX();++i){
			for(int j=(i==p.x?p.y+1:0);j<=min((long long)screen[i].size()-1,(long long)changeZone.maxY());++j){
				p={i,j};
				return i-x;
			}
		}
		p={-1,-1};
		return-1;
	}
	
	bool posLegal(const pos& p){//合法函数
		return p.x>=0&&p.y>=0&&p.x<screen.size()&&p.y<screen[p.x].size()&&(!changeZone.enable()||p.x>=changeZone.minX()&&p.y>=changeZone.minY()&&p.x<=changeZone.maxX()&&p.y<=changeZone.maxY());
	}

	char pushPxChar(const pxChar& px){
		return px.charColor==px.backColor?' ':px.c;
	}

	//中间函数区
	void setArrayPosition(const int& y=0,const int& x=0){//面向底层的数组设置，避免填入xy的争议
		setCursorPosition(x,y);
	}

	//核心功能区
	void setPx(int x,int y,char c){//修改指定位置的字符
		screen[x][y]={c,cursorPointer.charColor,cursorPointer.backColor};
		changeZone.push({x,y});
	}
	
	bool pxdif(const pxChar& a,const pxChar& b){//相差函数，一般用于比较旧像素与新像素 
		if((a.c==' '||a.charColor==a.backColor)&&(b.c==' '||b.charColor==b.backColor))return a.backColor!=b.backColor;
		return a.c!=b.c||a.charColor!=b.charColor||a.backColor!=b.backColor;
	}
	
	bool pxsim(const pxChar& a,const pxChar& b){//相似函数，一般用于维护 输出字符串 可一次输出 的 单调性
		if((a.c==' '||a.charColor==a.backColor)||(b.c==' '||b.charColor==b.backColor))return a.backColor==b.backColor;
		return a.charColor==b.charColor&&a.backColor==b.backColor;
	}

	pxChar pxspecial(const pxChar& a,const pxChar& b){//代表性函数，在相似的前提下返回更具有特殊性的那个像素（不相似返回异常像素）
		if(!pxsim(a,b))return{'\0',-1,-1};
		if(a.c==' '||a.charColor==a.backColor)return b;
		return a;
	}

	void outstring(string str){//提供给内部提供的输出处理函数
		vector<string>strs;
		string t="";
		for(int i=0;i<str.size();++i){
			if(str[i]=='\n')strs.push_back(t),t="";
			else if(str[i]=='\t')for(int j=0;j<tab_width;++j)t+=" ";
			else t+=str[i];
		}
		strs.push_back(t);

		if(screen.size()<cursorPointer.x+strs.size()){
			screen.resize(cursorPointer.x+strs.size(),vector<pxChar>{});
			tempScreen.resize(screen.size(),vector<pxChar>{});
		}

		for(int i=0;i<strs.size();++i){
			if(screen[cursorPointer.x].size()<=cursorPointer.y+strs[i].size()){
				screen[cursorPointer.x].resize(cursorPointer.y+strs[i].size(),{' ',7,0});
				tempScreen[cursorPointer.x].resize(screen[cursorPointer.x].size(),{' ',7,0});
			}
			for(int j=0;j<strs[i].size();++j)setPx(cursorPointer.x,cursorPointer.y,strs[i][j]),++cursorPointer.y;
			if(i<strs.size()-1)++cursorPointer.x,cursorPointer.y=0;
		}
	}
};
//实例化指令
FurryConsole::colorAtionCmd colorAtion(int charColor=7,int backColor=0){return{charColor,backColor};};
FurryConsole::setPositionCmd setPosition(int x=0,int y=0){return{x,y};};

FurryConsole cons; 

void cs1(){
	cons<<"hi!"<<"\n";
	cons<<"\n";
	cons<<"\n";
	cons<<"hi!";
	cons.setPosition(1,1);
	cons<<"py";
	cons.testCout();
}

void cs2(){
	for(int i=1000;i<=8000;++i)cons<<i<<"\n";
	cons.xrPlus();
}

void cs3(){
	cons<<colorAtion(7,1)<<"1";
	cons<<colorAtion(1,1)<<"1";
	cons<<colorAtion(6,1)<<"1";
	cons.xrPro();
}

void cs4(){
	cons<<colorAtion(7,1)<<"1";
	cons<<colorAtion(6,1)<<"1";
	cons<<colorAtion(7,1)<<"1\n";
	cons<<colorAtion(7,1)<<"1";
	cons<<colorAtion(7,1)<<"1";
	cons.xrPro();
}

void cs5(){
	cons<<colorAtion(6,1)<<"1";
	cons<<colorAtion(6,1)<<"1";
	cons<<colorAtion(7,1)<<"1\n";
	cons<<colorAtion(7,1)<<"1";
	cons<<colorAtion(7,1)<<"1";
	cons.xrPro();
}

//测试区 

const int N=10,M=50;

//波动函数（越靠后的返回，值越可能会大） 
long long cobdhs(){//颜色波动函数 
	//return 0;
	//return random(random(random(-1,0),0),random(0,random(0,1)));
	//return random(random(-1,0),random(0,1));
	//return random(-1,1);
	//return random(-2,2);
	return random(0,15);
}

long long cbdhs(int cmin=0,int cmax=0){//字符波动函数 
	//return 0;
	//return random(random(random(-1,0),0),random(0,random(0,1)));
	//return random(random(-1,0),random(0,1));
	//return random(-1,1);
	//return random(-2,2);
	return random(0,cmax-cmin);
}

void randomxg(){//随机初始化测试 
	const int csys=2;//测试模式 
	if(csys==1){//自定义波动测试
		cons.setPosition();
		int cc=7,bc=0;
		int cmin=32,cmax=127;
		char c='A';
		for(int i=0;i<N;++i){
			for(int j=0;j<M;++j){
				cc=((cc+cobdhs()&15)+16)&15;
				bc=((bc+cobdhs()&15)+16)&15;
				c=(((c+cbdhs(cmin,cmax)-cmin)%(cmax-cmin+1)+(cmax-cmin+1))%(cmax-cmin+1))+cmin;
				cons.colorAtion(cc,bc);
				cons<<c;
			}
			cons<<"\n";
		}
	}
	else if(csys==2){//大量相似、少数不相似测试 
		cons.setPosition();
		for(int i=0;i<N;++i){
			for(int j=0;j<M;++j){
				cons<<(char)random(32,127);
				cons.colorAtion(7,random(0,random(0,1)));
			}
			cons<<"\n";
		}
	}
}

void cs6(){//跑分测试 

	Sleep(1000);
	
	vector<long long>xrtime,xrProtime,xrPlustime;
	int n=100;
	
	system("cls");
	cons.init();
	for(int i=1;i<=n;++i){
		randomxg();
		long long s=GetTickCount();
		cons.xr();
		long long e=GetTickCount();
		xrtime.push_back(e-s);
	}
	
	system("cls");
	cons.init();
	for(int i=1;i<=n;++i){
		randomxg();
		long long s=GetTickCount();
		cons.xrPro();
		long long e=GetTickCount();
		xrProtime.push_back(e-s);
	}
	
	system("cls");
	cons.init();
	for(int i=1;i<=n;++i){
		randomxg();
		long long s=GetTickCount();
		cons.xrPlus();
		long long e=GetTickCount();
		xrPlustime.push_back(e-s);
	}
	
	//平均值统计 
	long long xrans=0,xrProans=0,xrPlusans=0;
	
	for(int i=0;i<n;++i){
		xrans+=xrtime[i];
		xrProans+=xrProtime[i];
		xrPlusans+=xrPlustime[i];
	}
	
	setCursorPosition(0,N);
	color_print();
	printf("xr:%.3lf\n",1.0*xrans/n/1000);
	printf("xrPro:%.3lf\n",1.0*xrProans/n/1000);
	printf("xrPlus:%.3lf\n",1.0*xrPlusans/n/1000);
}

void cs7(){
	vector<vector<int>>v={{1,2,3},{4,5,6},{7,8,9}};
	cons<<v<<"\n";
	map<string,int>mp;
	mp["Tom"]=13;
	mp["Eile"]=14;
	mp["Bob"]=12;
	cons<<mp<<"\n";
	cons.xrPro();
}

void cs8(){
	Sleep(1000);
	cons.setPosition();
	cons.colorAtion(7,0);
	for(int i=0;i<N;++i){
		for(int j=0;j<M;++j){
			cons<<"1";
		}
		cons<<"\n";
	}
	cons.xrPro();
	Sleep(1000);
	cons.setPosition();
	cons.colorAtion(7,0);
	for(int i=0;i<N;++i){
		for(int j=0;j<M;++j){
			if(j==0||j==M-1)cons.colorAtion(7,1);
			else cons.colorAtion(7,0);
			cons<<"1";
		}
		cons<<"\n";
	}
	cons.xrPro();
}

int main(){
	//cons.calibrateTabWidth();

	system("pause");
	color_print();
	cons.cls();
	cs1();

	system("pause");
	color_print();
	cons.cls();
	cs2();

	system("pause");
	color_print();
	cons.cls();
	cs3();

	system("pause");
	color_print();
	cons.cls();
	cs4();

	system("pause");
	color_print();
	cons.cls();
	cs5();

	system("pause");
	color_print();
	cons.cls();
	cs6();

	system("pause");
	color_print();
	cons.cls();
	cs7();

	system("pause");
	color_print();
	cons.cls();
	cs8();

	//cout<<color_str(7,0);
	system("pause");
	return 0;
}
