/*************************************************
* @brief Quebrando a cifra
* @author Igor Bernardo Daudt e Laura Seffrin Bagesteiro
* @date agosto 2022
Construir um decodificador de mensagens com criptografia XOR,
capaz de identificar a chave criptogr�fica.

retorno : Chave e texto decodificado em arquivo de sa�da
*************************************************/

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int qtd_chaves_possiveis = 10;     //!< quantidade chaves possiveis
char chaves_possiveis[] = "@#$%&*!+/-";

//! Enumerando erros possiveis durante a execucao do codigo
enum ERROS {  ERR_AB_ARQ = -3, //!< Erro abrindo o arquivo, arquivo nao existente
              ERR_CR_ARQ,     //!< Erro em criar o arquivo, abrir o arquivo criado
              ERR_CHV,       //!< Nao eh possivel resolver com xor / chave nao encontrada
              OK = 0        //!< Codigo ocorreu com sucesso
            };

int main(void)
{
    //!< declarando funcoes
    char retorna_chave(char linha[], int tam_linha, int pos_chave);
    void addsufixo(string &nome_arq);

    //!< declarando variaveis;
    ifstream infile;  //! arquivo de entrada
    ofstream outfile; //! arquivo de saida
    char chave[5];
    string nome_arq;   //!< string para receber o nome do arquivo pelo usuario

    cout << "         | Feito por Igor Bernardo Daudt e Laura Seffrin Bagesteiro |" << endl << endl;;
    cout << " |||Programa para descriptografar um arquivo de texto (.txt), que foi criptografado com uma chave XOR|||" << endl << endl;
    cout << " Digite o nome do arquivo a ser descriptografado, com ou sem o sufixo: ";
    cin >> nome_arq;
    addsufixo(nome_arq);

    //!< Abrindo o arquivo, verificando se ele existe e foi aberto
    infile.open( nome_arq , ios::in);
    if(!infile)
    {
        cerr << nome_arq << " nao pode ser aberto" << endl;
        return ERR_AB_ARQ;
    }

    //!< Conseguindo o tamanho do arquivo
    infile.seekg(0, infile.end);
    int tamanho = infile.tellg();
    infile.seekg(0, infile.beg);
    char *linha = new char[tamanho];  //!< variavel para receber todo o texto do arquivo

    //!< ler o arquivo, colocar ele em char linha[]
    infile.read(linha , tamanho);

    //!< Ver quantas linhas tem no arquivo, adaptar o tamanho
    for(int aux=0; aux<tamanho; aux++)
        if(linha[aux] == 10)  tamanho--;

    //!< conseguir a chave
    for(int i=0; i<4; i++)
    {
        chave[i] = retorna_chave(linha, tamanho, i);
        if(chave[i]== '0')
        {
            cerr << " Chave nao descoberta";
            return ERR_CHV;
        }
    }

    //!< ajustando o nome do arquivo e abrindo o arquivo de saida
    nome_arq = (nome_arq.substr(0, nome_arq.size()-4)) += "_dec.txt";
    outfile.open(nome_arq, ios::out);
    if(!outfile)
    {
        cerr << "Arquivo nao pode ser criado" << endl;
        return ERR_CR_ARQ;
    }

    //!< Colocando os resultados no arquivo de saida
    for(int i=0; i<tamanho; i++)
    {
        char printar = linha[i]^chave[i%4];
        outfile << printar;
    }
    outfile << endl << "utilizando a chave: " << chave;

    cout << "O resultado da decodificacao foi para o arquivo: " << nome_arq << endl;

    infile.close();
    outfile.close();

    return OK;
}

/***
* @brief adiciona o sufixo a string se necessario
* @param string contendo nome do arquivo dado pelo usuario
* @retval string com sufixo .txt
***/
void addsufixo(string &nome_arq)
{
    size_t encontrado = nome_arq.find(".txt");
    if(encontrado != string::npos);
    else nome_arq += ".txt";
}

/******
* @brief Retorna uma das chaves usadas para decodificar o texto
* @param texto do arquivo
* @param tamanho do arquivo
* @param posicao da chave a ser retornada
* @retval 1 caracter da chave  / 0 para chave nao encontrada
*
* For com int caracter faz passar chave por chave e fazer o calculo
XOR com os caracteres do texto, se o resultado deste calculo der, a toda
verificacao, um caracter da lingua portuguesa a funcao retornara a chave
como a correta
****/

char retorna_chave(char linha[], int tam_linha, int pos_chave)
{
    for(int caracter=0; caracter<qtd_chaves_possiveis; caracter++)
    {
        int checar = pos_chave; //!< variavel para contar de 4 em 4 quantas vezes foi comparado corretamente
        int ler = pos_chave;  //!< variavel para passar de 4 em 4 caracteres do texto, onde a mesma chave foi usada

        //! while para comparar a chave[] com todas posicoes onde ela possivelmente eh usada no arquivo
        while(ler < tam_linha)
        {
            char c_xor = chaves_possiveis[caracter]^linha[ler]; //!< variavel que armazena o calculo XOR
            //! verifica se eh um caracter valido, se correto modifica checar
            if(c_xor == 32 ||(c_xor>43 && c_xor<47)|| c_xor ==63 || (c_xor>64 && c_xor<91)|| (c_xor>96 && c_xor<123)||c_xor ==10 || c_xor ==13)
                checar+=4;
            ler += 4;
        }
        //! verifica se todos calculos xor deram caracteres validos
        if(checar == ler)
            return chaves_possiveis[caracter];
    }
    return '0'; //! retorna '0' para erro / chave nao encontrada
}

