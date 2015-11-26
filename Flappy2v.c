#include <msp430.h>

#define SCE  BIT5 // P2.5 LATCH
#define SDIN BIT7 // P1.7 DATA
#define SCLK BIT5 // P1.5 CLOCK
#define botao BIT4 //P1.4 BOTAO

#define PIN_DATA (BIT3 + BIT2 + BIT1 + BIT0) // D7 ... D4 Barramento P1
#define RS BIT1 //P2.1 RS
#define E BIT0  //P2.0 Enable

//###################################################################################################################


//variaveis usadas na interrupção de tempo, que escreve as matrizes que estão no programa nas de LEDs
int ponteiro = 0, ponteiro2 = 0, flagcor = 0; 
int linha_vermelha1 = 0, linha_vermelha2 = 0;
int linha_verde1 = 0, linha_verde2 = 0;

//matrizes que servem de interface entre o programa e o hardware
char Matrix_red[16][8];
char Matrix_green[16][8];

//na função que faz o shift das colunas, essa matriz armazena a nona coluna, que não esta sendo mostrada
char new_column[13];

//contador de colunas em branco 
int cont_column = 0;
//bit central representa as possibilidades de canos, bit central vai de 3 a 11, cada cano tem um intervalo de 4
int bit_central, bit_central_inst;

//coluna em que o flappy bird esta, se 0 ele esta colado no final da matriz
int col_flappy = 1;
int linha_atual_flappy;
//controla a interrupção da porta P1, é setada na interrupçao e quando esta setada faz o passaro subir uma casa
int flag_updown = 0;

//conta 10ms no loop
int count_10ms = 0;

//responsavel pela largura dos canos
int multiplicidade = 1;
int i = 1;

//enquanto flag_comeco se manter em 1, o programa fica travado
int flag_comeco = 1;

char placar[10];
int int_placar = 0;
int apagou = 1;
int pass_one = 0;
//###################################################################################################################

//declaração das funçẽos
//credito: https://github.com/wsilverio/exp-MSP430-C/tree/master/LCD_Nokia_PCD8544_SPI
void config_USIB(); 
//credito: https://github.com/wsilverio/exp-MSP430-C/tree/master/LCD_Nokia_PCD8544_SPI
void Write_Matrix(char, char, char, char, char, char); 
void delay_ms(unsigned int);
void configinter();    
void config_porta();
void config_int_porta1();
void perdeu();

//###################################################################################################################

void LCD_pulso_E();
void write_bin(char bin, char rs);
void inicializacao_LCD16x2();
void write_lcd(char *texto);
void itoa(long unsigned int inteiro, char* string);

//###################################################################################################################
void main(void){
  //desliga watchdog
  WDTCTL = WDTPW + WDTHOLD; 

  //clock 8MHz
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;

  config_porta();

  config_int_porta1();

  configinter();

  inicializacao_LCD16x2();

  write_lcd("Flappy Bird");
  write_bin(BIT7 + 0x40,0);
  write_lcd("By: Marco e Joao");

  delay_ms(200);


  //habilita interr
  _BIS_SR(GIE);

  config_USIB();

 // ###################################################################################################################


  //gera matrizes de zeros
  for(int i = 0; i < 16; ++i){
    for(int j = 0; j < 8; ++j){
      Matrix_green[i][j] = 0;
      Matrix_red[i][j] = 0;
    }
  }
    //geraçaõ campo, topo e piso
  for(int i = 0; i < 8; i++){
    Matrix_red[0][i] = 1; 
    Matrix_red[14][i] = 1;
    Matrix_red[15][i] = 1;
    Matrix_green[15][i] = 1; 
  }

    //posição inicial do passaro
  Matrix_green[9][col_flappy] = 1;

 //###################################################################################################################
 //###################################################################################################################
 //###################################################################################################################
 //###################################################################################################################

  while (1)
  { 
      //espera o botão ser apertado uma vez
      while(flag_comeco == 1){
        bit_central = TA1R + 2;
      }



    //geração de obstaculos
    //entra a cada 40ms
    if(count_10ms%4 == 0){


      if(cont_column >= 5){

        //bit_central = 4;

        //2 'for' para gerar os canos, baseados no bit central
        for (int i = 0; i < bit_central - 2; ++i){
          new_column[i] = 1;
        }

        for (int i = bit_central + 2; i < 13; ++i){
          new_column[i] = 1;
        }


        //repetir o msmo cano 2x
      if (i > multiplicidade){
        cont_column = 0;
        bit_central = bit_central_inst + 2;
        if (pass_one == 0)
        {
          int_placar--;
          pass_one = 1;
        }
        int_placar++;
        i = 0;
      }
        ++i;
      }

      //se nao tiver na hora de gerar mais obstaculos, a new_column é uma coluna de zeros
      else{
        for (int i = 0; i < 13; ++i){
          new_column[i] = 0;
        }
        cont_column++;
      }


      //faz o shift de uma coluna para a outra, por toda a matriz menos a ultima coluna
      for(int i = 0; i < 7; ++i){
        for (int i2 = 1; i2 < 14; ++i2){
          Matrix_red[i2][i] = Matrix_red[i2][i+1]; 
        }
      }
      //puxa para a matriz o valor de new_column
      for (int i = 1; i < 14; ++i){
        Matrix_red[i][7] = new_column[i-1];
      }
    }


    //movimentação flappy
    //entra a cada 20ms
    if(count_10ms%2==0){


      for (int i = 0; i < 14; ++i){
        if (Matrix_green[i][col_flappy] == 1){
          linha_atual_flappy = i;
        }
      }

      if (flag_updown == 0){
        Matrix_green[linha_atual_flappy][col_flappy] = 0;
        Matrix_green[linha_atual_flappy + 1][col_flappy] = 1;
      }

      else{
        Matrix_green[linha_atual_flappy][col_flappy] = 0;
        Matrix_green[linha_atual_flappy - 1][col_flappy] = 1;
        flag_updown = 0;
        delay_ms(5);

      }
    }  

    //verificação batida
    //verifica batida em canos 
    for (int i = 1; i < 13; ++i){
        if(Matrix_green[i][col_flappy] == 1 && Matrix_red[i][col_flappy] == 1){
          perdeu();
        }

    }
    //verifica batida no topo ou piso
    if(Matrix_green[0][col_flappy] == 1 || Matrix_green[14][col_flappy] == 1){
      perdeu();
    }




    delay_ms(10);
    count_10ms++;


//###################################################################################################################

    if (apagou == 1){
      write_bin(BIT7 + 0x00,0);
      write_lcd("Pontuacao:      ");
      write_bin(BIT7 + 0x40,0);
      write_lcd("                ");
      apagou = 0;
    }
    itoa(int_placar,placar);
    write_bin(BIT7 + 0x40,0);
    write_lcd(placar);
  }

}



void configinter(){
  TA0CTL |= TASSEL_2 + ID_0 + MC_1;        // INTERRUPÇÃO TIMER
  TA0CCTL0 |= CCIE;
  TA0CCR0 = 1000;

  TA1CTL |= TASSEL_2 + ID_0 + MC_1;        // INTERRUPÇÃO TIMER
  TA1CCR0 = 9;
}

void config_porta(){
  //configura P1 botao
  P1REN |= botao;
  P1OUT |= botao;

  P1DIR |= PIN_DATA;
  P1SEL &= ~PIN_DATA;
  P1SEL2 &= ~PIN_DATA;
  P1OUT &= ~PIN_DATA;

  //configura portas

 
  P2DIR |= (SCE + RS + E);
  P2OUT &= ~(RS + E);
}

void config_int_porta1(){
  //interrupção porta P1
  P1IE |= botao;             
  P1IES |= botao;        
  P1IFG = 0; 
}


//###################################################################################################################
//###################################################################################################################
//###################################################################################################################
__attribute__((interrupt(PORT1_VECTOR))) //interrupcao de botao para compilar com gcc
void P1(void){
  //flag para fazer subir 
  flag_updown = 1;
  //flag para fazer começar
  flag_comeco = 0;

  bit_central_inst = TA1R;



  //limpa flag interrupção
  P1IFG = 0;
}

__attribute__((interrupt(TIMER0_A0_VECTOR))) //interrupcao de timer para compilar com gcc
void Escrita_matriz(void){

  //acender so verde ou so vermelho de uma vez, se nao perde muita luminosidade
  //liga so o vermelho com flag = 0
  if(flagcor == 0){  

    if(ponteiro > 7){ //ponteiro aponta para linha de cada matriz, vai de 0 a 7
        ponteiro = 0;
      }
    else{

      for(int i = 0; i < 8; i++){
        linha_vermelha1 = linha_vermelha1 << 1; //transforma a linha em um numero só, para poder enviar para a matriz de led
        linha_vermelha1 += Matrix_red[ponteiro][7-i];
        linha_vermelha2 = linha_vermelha2 << 1;
        linha_vermelha2 += (Matrix_red[ponteiro+8][i]);
      }

      //escrita na matriz, essas alterações são para corrigir inversões feitas na placa
      Write_Matrix(128>>ponteiro,0 , ~linha_vermelha2,1<<ponteiro, 0, ~linha_vermelha1);
      ponteiro++;
    }
    
    flagcor = 1;
  }

  //faz a mesma coisa, so que com o verde agora
  else{
      if(ponteiro2 > 7){
        ponteiro2 = 0;
      }
    else{

      for(int i = 0; i < 8; i++){
        linha_verde1 = linha_verde1 << 1;
        linha_verde1 += Matrix_green[ponteiro2][7-i];
        linha_verde2 = linha_verde2 << 1;
        linha_verde2 += (Matrix_green[ponteiro2+8][i]);
      }

      Write_Matrix(128>>ponteiro2, linha_verde2, ~0, 1<<ponteiro2, linha_verde1, ~0);
      ponteiro2++;
    }
    flagcor = 0;
  }

  TA0CCTL0 &=~ CCIFG;  //reseta flag interr.
}

void config_USIB(){
    
    // USIB
    P1SEL |= SCLK + SDIN;
    P1SEL2 |= SCLK + SDIN;
  
    UCB0CTL1 |= UCSWRST;

    // Data capture, MSB, Master, 3-pin SPI, sinc
    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCMODE_0 + UCSYNC;
    // SMCLK
    UCB0CTL1 |= UCSSEL_2;
    // Clock
    UCB0BR0 = 1; // 8MHz / 1
    UCB0BR1 = 0;
    // SPI habilitada
    UCB0CTL1 &= ~UCSWRST;
  
    IFG2 &= ~UCB0TXIFG;
}

void Write_Matrix(char bin0, char bin1, char bin2, char bin3, char bin4, char bin5){

    P2OUT &= ~SCE;

    UCB0TXBUF = bin0;
  
    while(!(IFG2 & UCB0TXIFG));
    UCB0TXBUF = bin1;
  
    while(!(IFG2 & UCB0TXIFG));
    UCB0TXBUF = bin2;
  
    while(!(IFG2 & UCB0TXIFG));
    UCB0TXBUF = bin3;
  
    while(!(IFG2 & UCB0TXIFG));
    UCB0TXBUF = bin4;
  
    while(!(IFG2 & UCB0TXIFG));
    UCB0TXBUF = bin5;
  
    while(!(IFG2 & UCB0TXIFG));
  
    P2OUT |= SCE;
}


void perdeu(){ //quando perde, entre aqui, da uma piscada e redefine as condiçoes inciais - NA REINICIADA ESTA BUGANDO
  int tempoperdeu = 150;
    write_bin(BIT7 + 0x00,0);
    write_lcd("    GAME OVER     ");

   delay_ms(tempoperdeu);
  for(int i = 0; i < 16; ++i){
    for(int j = 0; j < 8; ++j){
       Matrix_green[i][j] = 0;
        Matrix_red[i][j] = 0;
    }
  }
  delay_ms(tempoperdeu);
  for(int i = 0; i < 16; ++i){
    for(int j = 0; j < 8; ++j){
       Matrix_green[i][j] = 1;
        Matrix_red[i][j] = 1;
    }
  }
  delay_ms(tempoperdeu);

  WDTCTL = 0;


}

void delay_ms(unsigned int ms){
    while(ms--){
        __delay_cycles(1000);
    }
}


//###################################################################################################################
//###################################################################################################################
//###################################################################################################################

void LCD_pulso_E(){
    // pulso enable (1ms)
    P2OUT &= ~E;
    P2OUT |= E;
    P2OUT &= ~E;
}

void write_bin(char bin, char rs){

    // RS = 0: cmd
    // RS = 1: dados
    P2OUT = (rs)?(P2OUT | RS):(P2OUT & ~RS);
    
    // zera o barramento
    P1OUT &= ~PIN_DATA;
    
    // escreve HSB
    P1OUT |= (0xF0 & bin) >> 4;

    // pulso enable (1ms)
    LCD_pulso_E();
    
    // zera o barramento
    P1OUT &= ~PIN_DATA;

    // escreve LSB
    P1OUT |= (0x0F & bin);

    // pulso enable (1ms)
    LCD_pulso_E();
}

void inicializacao_LCD16x2(){
    __delay_cycles(15000); // 15ms
    
    P1OUT |= 0x03;
    LCD_pulso_E(); //Escreve o HSB de 0x30 (necessario)
    __delay_cycles(5000); // 5ms
    
    LCD_pulso_E(); //Escreve o HSB de 0x30 (necessario)
    __delay_cycles(1000); // 1ms
    
    LCD_pulso_E(); //Escreve o HSB de 0x30 (necessario)
    __delay_cycles(1000); // 1ms
    
    // zera o barramento
    P1OUT &= ~PIN_DATA;

    P1OUT |= 0x02;    
    LCD_pulso_E(); //Escreve o HSB de 0x20 (necessario)    
    __delay_cycles(1000);  //1ms



    write_bin(0x28,0);
    __delay_cycles(1000); //1ms
    write_bin(0x08,0);
    __delay_cycles(1000); //1ms
    write_bin(0x01,0);
    __delay_cycles(1000); //1ms
    write_bin(0x06,0);
    __delay_cycles(1000); //1ms
    write_bin(0x0C,0);
}

void write_lcd(char *texto){
    while(*texto){
        write_bin(*texto, 1);
        texto++;
    }
}

void itoa(long unsigned int inteiro, char* string){
    
    char* ptr = string, *ptr1 = string, tmp_char;
    int tmp_inteiro;

    do{
        tmp_inteiro = inteiro;
        inteiro /= 10;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_inteiro - inteiro * 10)];
    }while(inteiro);
    
    if(tmp_inteiro < 0) *ptr++ = '-';
    
    *ptr-- = '\0';
    
    while(ptr1 < ptr){
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
}