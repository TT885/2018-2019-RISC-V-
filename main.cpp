#include <bits/stdc++.h>
using namespace std;
int i=0;
const int maxn=0x400000;//4M memory
class registers
{
public:
    unsigned int x[32];

    unsigned int lock[32],locknum[32],loadlock[32];
    unsigned int pc;
    unsigned int pclock,pcstop;
    registers(){pcstop=pclock=pc=x[0]=0; for(int i=0;i<32;++i) loadlock[i]=x[i]=lock[i]=0;}
    unsigned int &operator[](unsigned int rx)
    {
        return x[rx];
    }
}reg;
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
BEQ,BNE,BLT,BGE,BLTU,BGEU,SB,SH,SW};    

class instruction
{
    friend bool  ID(unsigned int instcode,instruction* &instp);
    unsigned int rd,rs1,rs2,imm,tmp1,tmp2,res,respc;
    bool flag;//jump
    OptionType       opttype;
    instruction_type insttype;
    inline void judgeend()
    {
        if(res==0x30004){
            cout<<dec<<(int)(reg[10]&255);
            exit(0);
        }
    }
public:
    instruction():flag(0),rd(0),rs1(0),rs2(0),res(0),tmp1(0),tmp2(0){}
    void show()
    {
        cout<<instructiontype[insttype]<<" ";
        switch(opttype)
        {
            case U:
            case J:cout<<hex<<"rd="<<regname[rd]<<" imm="<<imm<<"\n";break;
            case B:
            case S:cout<<hex<<"imm="<<imm<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";break;
            case I:cout<<hex<<"rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";break;
            case R:cout<<hex<<"rd="<<regname[rd]<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";break;
        }
        cout<<"tmp1="<<tmp1<<" tmp2="<<tmp2<<" res="<<res<<"\n";
    }
    inline void EX()
    {
        bool Load=0;
        if(reg.lock[rs2]) tmp2=reg.locknum[rs2];
        switch(insttype){
        case LUI:   res=imm;break;
        case AUIPC: res=respc+imm;break;
        case JAL:   res=respc+4;respc+=imm; flag=1;break;
        case JALR:  res=respc+4;respc=tmp1+imm;flag=1;break; 
        case LB:
        case LH:
        case LW:
        case LBU:
        case LHU:    Load=1;
        case SB:
        case SH:
        case SW:
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
        case BEQ:   flag=tmp1==tmp2;flag?respc+=imm:respc+=4,flag=1;break;
        case BNE:   flag=tmp1!=tmp2;flag?respc+=imm:respc+=4,flag=1;break;
        case BLT:   flag=(int)tmp1< (int)tmp2;flag?respc+=imm:respc+=4,flag=1;break;
        case BGE:   flag=(int)tmp1>=(int)tmp2;flag?respc+=imm:respc+=4,flag=1;break;
        case BLTU:  flag=(unsigned int)tmp1< (unsigned int)tmp2;flag?respc+=imm:respc+=4,flag=1;break;
        case BGEU:  flag=(unsigned int)tmp1>=(unsigned int)tmp2;flag?respc+=imm:respc+=4,flag=1;break;
        }
        if(rd && !Load) ++reg.lock[rd],reg.locknum[rd]=res; //B,S rd=0,Load is I type, rewrite in MA.
    }
    inline void MA()
    {
        switch(insttype){
        case LB: if(rd) res=mem[res],signedExtend(res,8),                              ++reg.lock[rd],reg.locknum[rd]=res;break;
        case LBU:if(rd) res=mem[res],                                                  ++reg.lock[rd],reg.locknum[rd]=res;break;
        case LH: if(rd) res=mem[res]+(mem[res+1]<<8),signedExtend(res,16),             ++reg.lock[rd],reg.locknum[rd]=res;break;
        case LHU:if(rd) res=mem[res]+(mem[res+1]<<8),                                  ++reg.lock[rd],reg.locknum[rd]=res;break;
        case LW: if(rd) res=mem[res]+(mem[res+1]<<8)+(mem[res+2]<<16)+(mem[res+3]<<24),++reg.lock[rd],reg.locknum[rd]=res;break;
        case SB: judgeend();mem[res]=tmp2&255;break;
        case SH: judgeend();mem[res]=tmp2&255;mem[res+1]=(tmp2>>8)&255;break;
        case SW: judgeend();mem[res]=tmp2&255;mem[res+1]=(tmp2>>8)&255;mem[res+2]=(tmp2>>16)&255;mem[res+3]=(tmp2>>24)&255;break;
        default:;
        }
        switch(insttype){
        case LB:
        case LBU:
        case LH:
        case LHU:
        case LW:     --reg.loadlock[rd];
        }
    }
    inline void WB()
    {
        if(opttype!=S && opttype!=B &&rd)
            reg[rd]=res,--reg.lock[rd];// 若下一条语句也改写了寄存器那么锁区仍然存在
        if(flag)            reg.pc=respc,  reg.pclock=0,reg.pcstop=1;//ID阶段还要跳转
    }
};

inline bool  ID (unsigned int instcode,instruction* &instp)
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
    break;
    case B:inst.rs1=(instcode>>15)&31;
    inst.rs2=(instcode>>20)&31;
    inst.imm=(instcode>>7)&30;inst.imm+=((instcode>>7)&1)<<11;inst.imm+=((instcode>>25)&63)<<5;inst.imm+=((instcode>>31)&1)<<12;
    signedExtend(inst.imm,12);
    break;
    case S: inst.rs1=(instcode>>15)&31;
    inst.rs2=(instcode>>20)&31;
    inst.imm=(instcode>>7)&31;inst.imm+=((instcode>>25)&127)<<5;//optcode抠掉！！
    signedExtend(inst.imm,11);
    break;
    case R: inst.rd=(instcode>>7)&31;
    inst.rs1=(instcode>>15)&31;
    inst.rs2=(instcode>>20)&31;
    inst.imm=inst.rs2;
    break;
    }
    if(reg.loadlock[inst.rs1] ||reg.loadlock[inst.rs2])
        return 0;


    if(optc==99 ||optc==103 ||optc==111)
        reg.pclock=1;
    if(!reg.pclock &&!reg.pcstop)
        reg.pc+=4;
    if(optc==3)
        ++reg.loadlock[inst.rd];
    inst.tmp1=reg.lock[inst.rs1]?reg.locknum[inst.rs1]:reg[inst.rs1];
    inst.tmp2=reg.lock[inst.rs2]?reg.locknum[inst.rs2]:reg[inst.rs2];
    instp=new instruction(inst);
    return 1;
}
int main()
{
    ios::sync_with_stdio(0);cin.tie(0);cout.tie(0);
    mem.getprogram();
    unsigned int qID[8],hID=0,hEX=0,hMA=0,hWB=0,tID=0,tEX=0,tMA=0,tWB=0;
    instruction *qEX[8],*qMA[8],*qWB[8],*inst;
    while(1){
        if(hWB!=tWB){
            qWB[hWB]->WB(),delete qWB[hWB],++hWB,hWB&=7;
        }
        if(hMA!=tMA){
            qMA[hMA]->MA(),qWB[tWB]=qMA[hMA],++tWB,tWB&=7,++hMA,hMA&=7;
        }
        if(hEX!=tEX){
            qEX[hEX]->EX(),qMA[tMA]=qEX[hEX],++tMA,tMA&=7,++hEX,hEX&=7;
        }
        if(hID!=tID && ID(qID[hID],inst)){
            qEX[tEX]=inst, ++tEX,tEX&=7,++hID,hID&=7;
        }
        if(!reg.pclock && hID==tID){
            qID[tID]=IF(reg.pc),++tID,tID&=7;
        }
        reg.pcstop=0;
    }

    return 0;
}
