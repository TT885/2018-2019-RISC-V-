#include <bits/stdc++.h>
using namespace std;

const int maxn=0x400000;//4M memory
class registers
{
public:
    unsigned int x[32];
    unsigned int pc;
    registers(){pc=x[0]=0;}
    unsigned int &operator[](unsigned int rx)
    {
        return x[rx];
    }
}reg;
string regname[32]={"zero","ra","sp","gp","tp","t0","t1","t2","s0/fp","s1","a0","a1","a2","a3","a4","a5","a6","a7","s2","s3","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","t6"};

class memory
{
public:
    unsigned char*  a;
    memory(){ a=new unsigned char[maxn]; for(int i=0;i<maxn;++i) a[i]=0;}
    ~memory() {delete []a;}
    unsigned char &operator[](unsigned int x)
    {
        return a[x];
    }
    void getprogram()
    {
        char inst[20];
        int position;
        while(cin>>inst){
            if(inst[0]=='@'){
                position=0;
                int len=strlen(inst);
                for(int i=1;i<len;++i)
                    position=(position<<4)+(inst[i]>='0'&&inst[i]<='9'?inst[i]-'0':10+inst[i]-'A'); //问题就在此处！
            }
            else{
                for(int i=0;i<2;++i)
                    a[position]=(a[position]<<4)+(inst[i]>='0'&&inst[i]<='9'?inst[i]-'0':10+inst[i]-'A');
                ++position;
            }
        }
    }
    void show()
    {
        for(int i=0;i<0x1288;++i) if(a[i]) cout<<hex<<i<<":"<<(int)a[i]<<" "<<dec;
    }
}mem;

inline unsigned int signedExtend(unsigned int &data,int bits)
{
    if((data>>bits)&1) data|=0xffffffff>>bits<<bits;
    return data;
}
inline unsigned int IF(int pc)
{
    return (mem[pc+3]<<24)+(mem[pc+2]<<16)+(mem[pc+1]<<8)+mem[pc];
}

enum OptionType{R,I,U,J,S,B};
enum instruction_type{LUI,AUIPC,JAL,   //3
JALR,LB,LH,LW,LBU,LHU,ADDI,SLTI,SLTIU,XORI,ORI,ANDI,SLLI,SRLI,SRAI,  //18
ADD,SUB,SLL,SLT,SLTU,XOR,SRL,SRA,OR,AND,    //28
BEQ,BNE,BLT,BGE,BLTU,BGEU,SB,SH,SW};    //37
string instructiontype[37]={"LUI","AUIPC","JAL",
"JALR","LB","LH","LW","LBU","LHU","ADDI","SLTI","SLTIU","XORI","ORI","ANDI","SLLI","SRLI","SRAI",  //18
"ADD","SUB","SLL","SLT","SLTU","XOR","SRL","SRA","OR","AND",
"BEQ","BNE","BLT","BGE","BLTU","BGEU","SB","SH","SW"};

class instruction
{
    friend instruction  ID(unsigned int instcode);
    unsigned int rd,rs1,rs2,imm,tmp1,tmp2,res,respc;
    bool flag;
    OptionType       opttype;
    instruction_type insttype;
    void judgeend()
    {
        if(res==0x30004){
            cout<<dec<<(int)(reg[10]&255);
            exit(0);
        }
    }
public:
    instruction():flag(0){}
    void show()
    {
        cout<<instructiontype[insttype]<<" ";

        //cerr<<"rd="<<rd<<" imm="<<imm<<" rs1="<<rs1<<" rs2="<<rs2<<"\n";
        switch(opttype)
        {
            case U:
            case J:cout<<hex<<"rd="<<regname[rd]<<" imm="<<imm<<"\n";break;
            case B:
            case S:cout<<hex<<"imm="<<imm<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";break;
            case I:cout<<hex<<"rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";break;
            case R:cout<<hex<<"rd="<<regname[rd]<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";break;
        }
    }
    void EX()
    {
        switch(insttype){
        case LUI:   res=imm;break;
        case AUIPC: res=respc+imm;break;
        case JAL:   res=respc+4;respc+=imm; flag=1;break;
        case JALR:  res=respc+4;respc=tmp1+imm;flag=1;break; //pc跳转方法和JAL完全不同
        case LB:
        case LH:
        case LW:
        case LBU:
        case LHU:
        case SB:
        case SH:
        case SW:    res=tmp1+imm;break;
        case ADDI:  res=tmp1+imm;break;
        case SLTI:  res=(int)tmp1<(int)imm;break;
        case SLTIU: res=(unsigned int)tmp1<(unsigned int)imm;break;
        case XORI:  res=tmp1^imm;break;
        case ORI:   res=tmp1|imm;break;
        case ANDI:  res=tmp1&imm;break;
        case SLLI:  res=tmp1<<imm;break;
        case SRLI:  res=(unsigned int)tmp1>>imm;break;
        case SRAI:  res=(int)tmp1>>imm;break;
        case ADD:   res=tmp1+tmp2;break;
        case SUB:   res=tmp1-tmp2;break;
        case SLL:   res=tmp1<<tmp2;break;
        case SLT:   res=tmp1<tmp2;break;
        case SLTU:  res=(unsigned int)tmp1<(unsigned int)tmp2;break;
        case XOR:   res=tmp1^tmp2;break;
        case SRL:   res=(unsigned int)tmp1>>tmp2;break;
        case SRA:   res=(int)tmp1>>tmp2;break;
        case OR:    res=tmp1|tmp2;break;
        case AND:   res=tmp1&tmp2;break;
        case BEQ:   flag=tmp1==tmp2;if(flag) respc+=imm;break;
        case BNE:   flag=tmp1!=tmp2;if(flag) respc+=imm;break;
        case BLT:   flag=(int)tmp1<(int)tmp2;if(flag) respc+=imm;break;
        case BGE:   flag=(int)tmp1>=(int)tmp2;if(flag) respc+=imm;break;
        case BLTU:  flag=(unsigned int)tmp1<(unsigned int)tmp2;if(flag) respc+=imm;break;
        case BGEU:  flag=(unsigned int)tmp1>=(unsigned int)tmp2;if(flag) respc+=imm;break;
        }
    }
    void MA()
    {
        //if((insttype==SB1 ||insttype==SH1||insttype==SW1) && res<=1000) cerr<<"warning!\n\n\n";
        switch(insttype){
        case LB: if(rd) res=mem[res],signedExtend(res,8);break;
        case LBU:if(rd) res=mem[res]; break;
        case LH: if(rd) res=mem[res]+(mem[res+1]<<8),signedExtend(res,16); break;
        case LHU:if(rd) res=mem[res]+(mem[res+1]<<8);break;
        case LW: if(rd) res=mem[res]+(mem[res+1]<<8)+(mem[res+2]<<16)+(mem[res+3]<<24);break;
        case SB: judgeend();mem[res]=tmp2&255;break;
        case SH: judgeend();mem[res]=tmp2&255;mem[res+1]=(tmp2>>8)&255;break;
        case SW: judgeend();mem[res]=tmp2&255;mem[res+1]=(tmp2>>8)&255;mem[res+2]=(tmp2>>16)&255;mem[res+3]=(tmp2>>24)&255;break;
        default:;
        }
    }
    void WB()
    {
        if(opttype!=S && opttype!=B &&rd)
            reg[rd]=res;
        reg.pc=(flag?respc:reg.pc+4);
    }
};

instruction  ID (unsigned int instcode)  //基类指针，多态实现
{
    instruction  inst;
    int optc=instcode&127,func=(instcode>>12)&7;
    switch(optc){
        case 55: inst.insttype= LUI;  inst.opttype=U;break;
        case 23: inst.insttype= AUIPC;inst.opttype=U;break;
        case 111:inst.insttype= JAL;  inst.opttype=J;break;
        case 103:inst.insttype= JALR; inst.opttype=I;break;
        case 99:switch(func){
            case 0:inst.insttype= BEQ;break;
            case 1:inst.insttype= BNE;break;
            case 4:inst.insttype= BLT;break;
            case 5:inst.insttype= BGE;break;
            case 6:inst.insttype= BLTU;break;
            case 7:inst.insttype= BGEU;break;
        }inst.opttype=B;break;
        case 3:switch(func){
            case 0:inst.insttype= LB;break;
            case 1:inst.insttype= LH;break;
            case 2:inst.insttype= LW;break;
            case 4:inst.insttype= LBU;break;
            case 5:inst.insttype= LHU;break;
        }inst.opttype=I;break;
        case 35:switch(func){
            case 0:inst.insttype= SB;break;
            case 1:inst.insttype= SH;break;
            case 2:inst.insttype= SW;break;
        }inst.opttype=S;break;
        case 19:switch(func){
            case 0:inst.insttype= ADDI;break;
            case 2:inst.insttype= SLTI;break;
            case 3:inst.insttype= SLTIU;break;
            case 4:inst.insttype= XORI;break;
            case 6:inst.insttype= ORI;break;
            case 7:inst.insttype= ANDI;break;
            case 1:inst.insttype= SLLI;break;
            case 5:(instcode>>30)?inst.insttype= SRAI:inst.insttype= SRLI;break;
        }inst.opttype=I;break;
        case 51:switch(func){
            case 0:(instcode>>30)?inst.insttype= SUB:inst.insttype= ADD;break;
            case 1:inst.insttype= SLL; break;
            case 2:inst.insttype= SLT; break;
            case 3:inst.insttype= SLTU;break;
            case 4:inst.insttype= XOR; break;
            case 5:(instcode>>30)?inst.insttype= SRA:inst.insttype= SRL;break;
            case 6:inst.insttype= OR; break;
            case 7:inst.insttype= AND;break;
        }inst.opttype=R;break;
    }
    inst.respc=reg.pc;
    switch(inst.opttype){
    case U: inst.rd=(instcode>>7)&31;
    inst.imm=((instcode>>12)&0xfffff)<<12;
    break;
    case J: inst.rd=(instcode>>7)&31;
    inst.imm= (instcode>>20)&2046;         inst.imm+=((instcode>>20)&1)<<11;
    inst.imm+=((instcode>>12)&255)<<12;    inst.imm+=((instcode>>31)&1)<<20;
    signedExtend(inst.imm,20);
    break;
    case I: inst.rd=(instcode>>7)&31;
    inst.rs1=(instcode>>15)&31;
    inst.imm=(instcode>>20)&4095;
    signedExtend(inst.imm,11);
    inst.tmp1=reg[inst.rs1];
    break;
    case B:inst.rs1=(instcode>>15)&31;
    inst.rs2=(instcode>>20)&31;
    inst.imm=(instcode>>7)&30;inst.imm+=((instcode>>7)&1)<<11;inst.imm+=((instcode>>25)&63)<<5;inst.imm+=((instcode>>31)&1)<<12;
    signedExtend(inst.imm,12);
    inst.tmp1=reg[inst.rs1];
    inst.tmp2=reg[inst.rs2];
    break;
    case S: inst.rs1=(instcode>>15)&31;
    inst.rs2=(instcode>>20)&31;
    inst.imm=(instcode>>7)&31;inst.imm+=((instcode>>25)&127)<<5;//optcode抠掉！！
    signedExtend(inst.imm,11);
    inst.tmp1=reg[inst.rs1];
    inst.tmp2=reg[inst.rs2];
    break;
    case R: inst.rd=(instcode>>7)&31;
    inst.rs1=(instcode>>15)&31;
    inst.rs2=(instcode>>20)&31;
    inst.imm=inst.rs2;
    inst.tmp1=reg[inst.rs1];
    inst.tmp2=reg[inst.rs2];
    break;
    }
    return inst;
}

int main()
{
    ios::sync_with_stdio(0);cin.tie(0);cout.tie(0);
    //freopen("src/expr.data","r",stdin);
    //freopen("src/pi.data","r",stdin);
    //freopen("src/array_test1.data","r",stdin);
    //freopen("out.txt","w",stdout);

    mem.getprogram();
    while(1){
        unsigned int instcode=IF(reg.pc);
        instruction  inst=ID(instcode);

        //cerr<<hex<<reg.pc<<":"<<instcode<<"\t";inst.show();for(int i=0;i<32;++i) cout<<reg[i]<<" ";cout<<endl;
        inst.EX();
        inst.MA();
        inst.WB();
    }
    return 0;
}
