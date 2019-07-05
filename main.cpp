#include <bits/stdc++.h>
using namespace std;
//指令名称，rs1,rs2,rd解析基本正确，立即数解析错误

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
    unsigned char* a;
    memory(){ a=new unsigned char[maxn]; for(int i=0;i<maxn;++i) a[i]=0;}
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

/*enum instruction_type{LUI,AUIPC,JAL,   //3
JALR,LB,LH,LW,LBV,LHU,ADDI,SLTI,SLTIU,XORI,ORI,ANDI,SLLI,SRLI,SRAI,  //18
ADD,SUB,SLL,SLT,SLTU,XOR,SRL,SRA,OR,AND,    //28
BEQ,BNE,BLT,BGE,BLTU,BGEU,SB,SH,SW};    //37
//*/

class instruction
{
    friend instruction* parse(unsigned int instcode);
protected:
    unsigned int rd,rs1,rs2,imm;
public:
    virtual int EX()=0;
    virtual void show()=0;
};

class LUI:public instruction
{
public:
    int EX()
    {
        reg[rd]=imm;
        //cerr<<"寄存器得到的值："<<reg[rd]<<"\n";
        return 1; //是否跳转
    }
    void show(){cout<<"LUI rd="<<regname[rd];cout<<hex<<" imm="<<imm<<"\n"<<dec;}
};

class AUIPC:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=imm+reg.pc;
        return 1; //是否跳转
    }
    void show(){cout<<"AUIPC rd="<<regname[rd];cout<<hex<<" imm="<<imm<<"\n"<<dec;}
};
class JAL:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg.pc+4;
        reg.pc+=imm;
        return 0; //是否跳转
    }
    void show(){cout<<"JAL rd="<<regname[rd];cout<<hex<<" imm="<<imm<<"\n"<<dec;}
};
class JALR:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg.pc+4;
        reg.pc=reg[rs1]+imm;
        return 0; //是否跳转
    }
    void show(){cout<<"JALR rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};

class BEQ:public instruction
{
    int EX()
    {
        if(reg[rs1]==reg[rs2]){
            reg.pc+=imm;return 0;
        }
        return 1; //是否跳转
    }
    void show(){cout<<"BEQ imm="<<imm<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";}
};

class BNE:public instruction
{
    int EX()
    {
        if(reg[rs1]!=reg[rs2]){
            reg.pc+=imm;return 0;
        }
        return 1; //是否跳转
    }
    void show(){cout<<"BNE imm="<<imm<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";}
};
class BGE:public instruction
{
    int EX()
    {
        if((int)reg[rs1]>=(int)reg[rs2]){
            reg.pc+=imm;return 0;
        }
        return 1;
    }
    void show(){cout<<"BGE imm="<<imm<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";}
};

class BGEU:public instruction
{
    int EX()
    {
        if((unsigned)reg[rs1]>=(unsigned)reg[rs2]){
            reg.pc+=imm;return 0;
        }
        return 1;
    }
    void show(){cout<<"BGEU imm="<<imm<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";}
};
class BLT:public instruction
{
    int EX()
    {
        if((int)reg[rs1]<(int)reg[rs2]){
            reg.pc+=imm;return 0;
        }
        return 1;
    }
    void show(){cout<<"BLT imm="<<imm<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";}
};
class BLTU:public instruction
{
    int EX()
    {
        if((unsigned)reg[rs1]<(unsigned)reg[rs2]){
            reg.pc+=imm;return 0;
        }
        return 1;
    }
    void show(){cout<<"BLTU imm="<<imm<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";}
}; //B类型

class LB:public instruction
{
    int EX()
    {
        if(rd){
            reg[rd]=mem[reg[rs1]+imm];
            signedExtend(reg[rd],8);
        }
        return 1;
    }
    void show(){cout<<"LB rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class LBU:public instruction
{
    int EX()
    {
        if(rd)
            reg[rd]=mem[reg[rs1]+imm];
        return 1;
    }
    void show(){cout<<"LBU rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class LH:public instruction
{
    int EX()
    {
        if(rd){
            reg[rd]=mem[reg[rs1]+imm]+(mem[reg[rs1]+imm+1]<<8);
            signedExtend(reg[rd],16);
        }
        return 1;
    }
    void show(){cout<<"LH rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class LHU:public instruction
{
    int EX()
    {
        if(rd)
            reg[rd]=mem[reg[rs1]+imm]+(mem[reg[rs1]+imm+1]<<8);
        return 1;
    }
    void show(){cout<<"LHU rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class LW:public instruction
{
    int EX()
    {
        if(rd)
            reg[rd]=mem[reg[rs1]+imm]+(mem[reg[rs1]+imm+1]<<8)+(mem[reg[rs1]+imm+2]<<16)+(mem[reg[rs1]+imm+3]<<24);

        return 1;
    }
    void show(){cout<<"LW rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};












class SB:public instruction
{
    int EX()
    {
        int po=reg[rs1]+imm;
        if(po==0x30004){
            cout<<(int)(reg[10]&255);//Wrong
            exit(0);
        }
        mem[po]=reg[rs2]&255;
        return 1;
    }
    void show(){cout<<"SB imm="<<imm<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";}
};
class SH:public instruction
{
    int EX()
    {
        int po=reg[rs1]+imm;
        if(po==0x30004){
            cout<<(int)(reg[10]&255);//Wrong
            exit(0);
        }
        mem[po]=reg[rs2]&255;mem[po+1]=(reg[rs2]>>8)&255;
        return 1;
    }
    void show(){cout<<"SH imm="<<imm<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";}
};
class SW:public instruction
{
    int EX()
    {
        int po=reg[rs1]+imm;
        if(po==0x30004){
            cout<<(int)(reg[10]&255);//Wrong
            exit(0);
        }
        mem[po]=reg[rs2]&255;mem[po+1]=(reg[rs2]>>8)&255;mem[po+2]=(reg[rs2]>>16)&255;mem[po+3]=(reg[rs2]>>24)&255;
        return 1;
    }
    void show(){cout<<"SW imm="<<imm<<" rs1="<<regname[rs1]<<" rs2="<<regname[rs2]<<"\n";}
};












class ADDI:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]+imm;
        return 1;
    }
    void show(){cout<<"ADDI rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class ADD:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]+reg[rs2];
        return 1;
    }

    void show(){cout<<"ADD rd="<<regname[rd]<<" rs2="<<regname[rs2]<<" rs1="<<regname[rs1]<<"\n";}
};
class SUB:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]-reg[rs2];
        return 1;
    }
    void show(){cout<<"SUB rd="<<regname[rd]<<" rs2="<<regname[rs2]<<" rs1="<<regname[rs1]<<"\n";}

};
class ANDI:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]&imm;
        return 1;
    }
    void show(){cout<<"ANDI rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class AND:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]&reg[rs2];
        return 1;
    }
    void show(){cout<<"AND rd="<<regname[rd]<<" rs2="<<regname[rs2]<<" rs1="<<regname[rs1]<<"\n";}
};
class XORI:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]^imm;
        return 1;
    }
    void show(){cout<<"XORI rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class XOR:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]^reg[rs2];
        return 1;
    }
    void show(){cout<<"XOR rd="<<regname[rd]<<" rs2="<<regname[rs2]<<" rs1="<<regname[rs1]<<"\n";}

};
class ORI:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]|imm;
        return 1;
    }
    void show(){cout<<"ORI rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class OR:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]|reg[rs2];
        return 1;
    }
    void show(){cout<<"OR rd="<<regname[rd]<<" rs2="<<regname[rs2]<<" rs1="<<regname[rs1]<<"\n";}
};
class SLLI:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]<<imm;
        return 1;
    }
    void show(){cout<<"SLLI rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class SLL:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]<<reg[rs2];
        return 1;
    }
    void show(){cout<<"SLL rd="<<regname[rd]<<" rs2="<<regname[rs2]<<" rs1="<<regname[rs1]<<"\n";}
};
class SRLI:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]>>imm;
        return 1;
    }
    void show(){cout<<"SRLI rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class SRL:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=reg[rs1]>>reg[rs2];
        return 1;
    }
    void show(){cout<<"SRL rd="<<regname[rd]<<" rs2="<<regname[rs2]<<" rs1="<<regname[rs1]<<"\n";}
};
class SRAI:public instruction //算术右移，负数左边补1
{
    int EX()
    {
        if(rd) reg[rd]=(int)reg[rs1]>>imm; //会先类型准换的
        return 1;
    }
    void show(){cout<<"SRAI rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class SRA:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=(int)reg[rs1]>>reg[rs2];
        return 1;
    }
    void show(){cout<<"SRA rd="<<regname[rd]<<" rs2="<<regname[rs2]<<" rs1="<<regname[rs1]<<"\n";}
};
class SLTI:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=(int)reg[rs1]<(int)imm;
        return 1;
    }
    void show(){cout<<"SLTI rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class SLT:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=(int)reg[rs1]<(int)reg[rs2];
        return 1;
    }
    void show(){cout<<"SLT rd="<<regname[rd]<<" rs2="<<regname[rs2]<<" rs1="<<regname[rs1]<<"\n";}
};
class SLTIU:public instruction
{
    int EX()
    {
        if(rd) reg[rd]=(unsigned int)reg[rs1]<(unsigned int)imm;
        return 1;
    }
    void show(){cout<<"SLTIU rd="<<regname[rd]<<" imm="<<imm<<" rs1="<<regname[rs1]<<"\n";}
};
class SLTU:public instruction
{
    int EX()
    {
        if(rd)  reg[rd]=(unsigned int)reg[rs1]<(unsigned int)reg[rs2];
        return 1;
    }
    void show(){cout<<"SLTU rd="<<regname[rd]<<" rs2="<<regname[rs2]<<" rs1="<<regname[rs1]<<"\n";}
};

instruction* parse (unsigned int instcode)  //基类指针，多态实现
{
    instruction *inst;
    int optc=instcode&127,func=(instcode>>12)&7;
    enum {R,I,U,J,S,B} opttype;
    switch(optc){
        case 55: inst=new LUI;  opttype=U;break;
        case 23: inst=new AUIPC;opttype=U;break;
        case 111:inst=new JAL;  opttype=J;break;
        case 103:inst=new JALR; opttype=I;break;
        case 99:switch(func){
            case 0:inst=new BEQ;break;
            case 1:inst=new BNE;break;
            case 4:inst=new BLT;break;
            case 5:inst=new BGE;break;
            case 6:inst=new BLTU;break;
            case 7:inst=new BGEU;break;
        }opttype=B;break;
        case 3:switch(func){
            case 0:inst=new LB;break;
            case 1:inst=new LH;break;
            case 2:inst=new LW;break;
            case 4:inst=new LBU;break;
            case 5:inst=new LHU;break;
        }opttype=I;break;
        case 35:switch(func){
            case 0:inst=new SB;break;
            case 1:inst=new SH;break;
            case 2:inst=new SW;break;
        }opttype=S;break;
        case 19:switch(func){
            case 0:inst=new ADDI;break;
            case 2:inst=new SLTI;break;
            case 3:inst=new SLTIU;break;
            case 4:inst=new XORI;break;
            case 6:inst=new ORI;break;
            case 7:inst=new ANDI;break;
            case 1:inst=new SLLI;break;
            case 5:(instcode>>30)?inst=new SRAI:inst=new SRLI;break;
        }opttype=I;break;
        case 51:switch(func){
            case 0:(instcode>>30)?inst=new SUB:inst=new ADD;break;
            case 1:inst=new SLL; break;
            case 2:inst=new SLT; break;
            case 3:inst=new SLTU;break;
            case 4:inst=new XOR; break;
            case 5:(instcode>>30)?inst=new SRA:inst=new SRL;break;
            case 6:inst=new OR; break;
            case 7:inst=new AND;break;
        }opttype=R;break;
    }

    switch(opttype){
    case U: inst->rd=(instcode>>7)&31;
    inst->imm=((instcode>>12)&0xfffff)<<12;break;

    case J: inst->rd=(instcode>>7)&31;
    inst->imm= (instcode>>20)&2046;         inst->imm+=((instcode>>20)&1)<<11;
    inst->imm+=((instcode>>12)&255)<<12;    inst->imm+=((instcode>>31)&1)<<20;
    //cerr<<hex<<((instcode>>21)&1023)<<" "<<(((instcode>>20)&1)<<11)<<" "<<(((instcode>>12)&255)<<12)<<" "<<(((instcode>>31)&1)<<20)<<endl;
    signedExtend(inst->imm,20);break;

    case I: inst->rd=(instcode>>7)&31;
    inst->rs1=(instcode>>15)&31;
    inst->imm=(instcode>>20)&4095;
    signedExtend(inst->imm,11);break;

    case B:inst->rs1=(instcode>>15)&31;
    inst->rs2=(instcode>>20)&31;
    inst->imm=(instcode>>7)&30;inst->imm+=((instcode>>7)&1)<<11;inst->imm+=((instcode>>25)&63)<<5;inst->imm+=((instcode>>31)&1)<<12;
    signedExtend(inst->imm,12);break;

    case S: inst->rs1=(instcode>>15)&31;
    inst->rs2=(instcode>>20)&31;
    inst->imm=(instcode>>7)&31;inst->imm+=((instcode>>25)&127)<<5;//optcode抠掉！！
    signedExtend(inst->imm,11);break;

    case R: inst->rd=(instcode>>7)&31;
    inst->rs1=(instcode>>15)&31;
    inst->rs2=(instcode>>20)&31;
    inst->imm=inst->rs2;break;/*for SRLI SLLI SRAI only*/
    }
    return inst;
}

int main()
{
    //freopen("src/expr.data","r",stdin);
    //freopen("src/naive.data","r",stdin);
    //freopen("src/array_test1.data","r",stdin);
    mem.getprogram();
    //mem.show();
    while(1){
        unsigned int instcode=IF(reg.pc);
        instruction *inst=parse(instcode);
        //cerr<<hex<<reg.pc<<":"<<instcode<<"\t";inst->show();
        if(inst->EX())  reg.pc+=4;
    }
    return 0;
}
