#include<bits/stdc++.h>
#include<windows.h>
#include<conio.h>
using std::cout;
using std::cin;
using std::min;
using std::max;
using std::string;
using std::ostream;
using std::vector;
using std::map;
using std::stringstream;

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
ostream& operator<<(ostream& os, const map<T1,T2>& mp){
	bool pd=false;
	os<<"{";
	for(typename map<T1,T2>::const_iterator it=mp.begin();it!=mp.end();os<<it->first<<":"<<it->second,++it,pd=true)if(pd)os<<",";
	os<<"}";
	return os;
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

	//API指令区
	struct colorAtionCmd{//变色指令
		int charColor,backColor;
	};

	struct setPositionCmd{//设置光标位置指令
		int x,y;
	};
	
	//关键函数：类的实际作用与外部交互的函数

	//渲染器函数（静态数组版）
	/*
	void xrPro(){//正在维护的渲染器 
		cursor tempCursor={0,0,7,0},xrzz=tempCursor;
		string outstr="";
		pxChar* ts=&tempScreen[0][0];
		pxChar* s=&screen[0][0];
		setArrayPosition();
		color_print();
		for(int i=0;i<N*M;){
			if(pxdif(*ts,*s)){//如果与旧像素的不一样 
				outstr="",tempCursor={i/M,i%M,(*s).charColor,(*s).backColor};//设置起始渲染状态 
				int xrzzx=xrzz.x,xrzzy=xrzz.y;//分离x、y更新 
				while(i<N*M&&pxsim({'\0',tempCursor.charColor,tempCursor.backColor},*s)
					//下面的注释可选，一般看情况 
					//&&pxdif(*ts,*s)
					){//维护outstr的输出单调性
					outstr+=(*s).charColor==(*s).backColor?' ':(*s).c;//拼接 
					if(i%M==M-1)outstr+="\n",++xrzzx,xrzzy=0;//换行的情况 
					else ++xrzzy;//一般的情况
					*ts=*s,++i,++ts,++s;//自增 
				}
				if(xrzz.x!=tempCursor.x||xrzz.y!=tempCursor.y)setArrayPosition(tempCursor.x,tempCursor.y);//调整光标位置（如果与原位置相同则不调用系统级别的API） 
				if(xrzz.charColor!=tempCursor.charColor||xrzz.backColor!=tempCursor.backColor)color_print(tempCursor.charColor,tempCursor.backColor);//调整输出颜色（如果与原位置相同则不调用系统级别的API） 
				cout<<outstr;//输出单调性字符串 
				xrzz={xrzzx,xrzzy,tempCursor.charColor,tempCursor.backColor};//更新输出完后光标信息 
			}
			else ++i,++ts,++s;//自增 
		}
	}
	*/

	void xr(){//老一套的无脑渲染器，多用作跑分比较
		setArrayPosition();
		for(int i=0;i<screen.size();++i){
			for(int j=0;j<screen[i].size();++j){
				color_print(screen[i][j].charColor,screen[i][j].backColor);
				cout<<screen[i][j].c;
			}
			cout<<"\n";
		}
		tempScreen=screen;
	}

	void testCout(){
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
	
	struct pos{
		int x,y;
	};

	//渲染器函数（动态数组版）
	void xrPro(){//正在维护的渲染器
		cursor xrzz={-1,-1,-1,-1};//光标指针
		for(pos p=firstpos();posLegal(p);){
			if(pxdif(tempScreen[p.x][p.y],screen[p.x][p.y])){
				string out="",undout="";
				pxChar special=screen[p.x][p.y];
				pos temppos=p;
				int x=p.x,y=p.y;
				while(posLegal(p)&&pxsim(special,screen[p.x][p.y])){
					if(pxdif(tempScreen[p.x][p.y],screen[p.x][p.y]))out+=undout+pushPxChar(screen[p.x][p.y]),y+=undout.size()+1,undout="";
					else undout+=pushPxChar(screen[p.x][p.y]);
					tempScreen[p.x][p.y]=screen[p.x][p.y],special=pxspecial(special,screen[p.x][p.y]);
					int line=addpos(p);
					if(line>0)out+=string("\n")*line,x+=line,y=0,undout="";
				}
				if(xrzz.x!=temppos.x||xrzz.y!=temppos.y)setArrayPosition(temppos.x,temppos.y);
				if(xrzz.charColor!=special.charColor||xrzz.backColor!=special.backColor)color_print(special.charColor,special.backColor);
				cout<<out,xrzz={x,y,special.charColor,special.backColor};
			}
			else addpos(p);
		}
	}

private://内部使用

	pos firstpos(){
		for(int i=0;i<screen.size();++i)if(screen[i].size())return{i,0};
		return{-1,-1};
	}

	int addpos(pos& p){
		for(int i=p.x,x=p.x;i<screen.size();++i){
			for(int j=(i==p.x?p.y+1:0);j<screen[i].size();++j){
				p={i,j};
				return i-x;
			}
		}
		p={-1,-1};
		return-1;
	}
	
	bool posLegal(const pos& p){//合法函数
		return p.x>=0&&p.y>=0&&p.x<screen.size()&&p.y<screen[p.x].size();
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

		if(screen.size()<=cursorPointer.x+strs.size()-1){
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
	for(int i=1;i<=10000;++i)cons<<i<<"\n";
	cons.xrPro();
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
	const int csys=1;//测试模式 
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
	
	vector<long long>xrtime,xrProtime;
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
	
	//平均值统计 
	long long xrans=0,xrProans=0;
	
	for(int i=0;i<n;++i){
		xrans+=xrtime[i];
		xrProans+=xrProtime[i];
	}
	
	setCursorPosition(0,N);
	color_print();
	printf("xr:%.3lf\n",1.0*xrans/n/1000);
	printf("xrPro:%.3lf\n",1.0*xrProans/n/1000);
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
	cs6();
	system("pause");
	return 0;
}
