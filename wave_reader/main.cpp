/***********************************************************
* @brief Pegando onda
* @author Igor Bernardo Daudt e Laura Seffrin Bagesteiro
* @date setembro 2022
* Construir um programa para windowns, modo console. dar ao
* usuario a escolha de, ou mostrar as informacoes do arquivo
* wav ou corta-lo.
*
* retorno : Informacoes do audio e/ou audio cortado
************************************************************/
#include <iostream>
#include <fstream>
#include <locale>

using namespace std;

enum progERR_t{  ERR_AB_ARQ = -5, //erro em abrir arquivo
                 ERR_CR_ARQ,  //erro em criar arquivo
                 ERR_WAV,    // arquivo nao eh wave
                 ERR_PAR,  // fora dos parametros do projeto
                 ERR_ALLOC, // erro na alocacao
                 OK};  // sucesso na execucao

struct wavCHK1_t {
            char id[4]; // RIFF
            uint32_t tam; // tamanho do arquivo - 8
            char fmt[4]; // WAVE
                };
struct wavCHK2_t{
            char          fmt[4];         // FMT
            uint32_t      Subchunk1Size;  // tamanho desse chunk
            uint16_t      audio_format;    // Formato do audio
            uint16_t      num_chan;      // Numero de canais
            uint32_t      sample_sec;  // amostras por segundo
            uint32_t      bytes_sec;    // bytes por segundo
            uint16_t      block_allign;     // 2=16-bit mono, 4=16-bit stereo
            uint16_t      bits_per_sample;  // Numero de bits por amostra
                };
struct wavCHK3_t{
            char id_data[4]; // data
            uint32_t tam_data; // tamanho do data chunk
            uint16_t *data = nullptr; // amostras de audio
                };
struct header_METADATA{
            char list_id[4]; //LIST
            uint32_t tam_meta; //tamanho dos metadados
            char tipo_info[4]; //INFO
};
struct wav_META_INFO{
            char info_id[4]; // tipo de info contida no metadado
            uint32_t tam_info; // tamanho da informacao no metadado
            char *detalhe = nullptr; // informacao em texto
};

//! variavel para carregar a quantidade de metadados totais no arquivo
int total_caracteres_metadados;

int main(void)
{
    //! Declarando funcoes
    void show_info(wavCHK1_t &audio, wavCHK2_t &info_audio);
    void menu_principal(void);
    void addsufixo(string &nome_arq);
    void cortar_audio(wavCHK2_t base,wavCHK3_t &audio, ofstream &outfile);

    //! permitindo acentos e caracteres da lingua portuguesa
    setlocale(LC_ALL,"");

    //! declarando variaveis
    string filename; // nome do arquivo dado pelo usuario
    string teste_wave; // Testar se o arquivo eh WAVE
    int escolha_usuario = 1; // variavel que carrega a escolha do usuario
    int qtd_metadata = 0; // quantidade de informacoes de metadados
    wavCHK1_t wcid; // struct do chunk descriptor
    wavCHK2_t format; // struct do fmt subchunk
    wavCHK3_t som;    // struct do data subchunk
    header_METADATA meta; // struct dos metadados
    wav_META_INFO lista[22]; // vetor da struct para conter variadas info sobre o audio
    ifstream inFile; // inFile � o arquivo de leitura dos dados
    ofstream outfile; // arquivo para audio recortado
    char completar_meta[500]; // variavel para completar os metadados faltando

    cout << "  | Feito por Igor Bernardo Daudt e Laura Seffrin Bagesteiro |" << endl << endl;;
    cout << " |||Programa para manipular arquivos no formato WAV |||" << endl;
    cout << " Nome do arquivo a ser analisado: ";
    cin >> filename;
    addsufixo(filename); // adiciona sufixo se necessario

    //! Abrindo arquivo, verificando por erros
    inFile.open(filename, ios::binary);
    if(!inFile)
    {
        cerr << filename << " n�o pode ser aberto" << endl;
        return ERR_AB_ARQ;
    }

    //! Lendo arquivo, colocando-o nas variaveis
    inFile.read((char *)&wcid , sizeof(wavCHK1_t)); // lendo o chunk descriptor
    inFile.read((char *) &format, sizeof(wavCHK2_t)); // lendo o fmt subchunk

    //! Verificando se o arquivo � .wav
    teste_wave = wcid.fmt;
    teste_wave = teste_wave.substr(0, 4);
    if(teste_wave != "WAVE")
    {
        cerr << "Arquivo n�o � do formato wav" << endl;
        return ERR_WAV;
    }

    //! Verificando se o arquivo � audio mono e PCM 16 bits
    if(format.audio_format != 1)
    {
        cerr << "arquivo n�o � do tipo PCM 16 bits" << endl;
        return ERR_PAR;
    }
    if(format.num_chan != 1)
    {
        cerr << "arquivo n�o possui apenas um canal de audio" << endl;
        return ERR_PAR;
    }

    //! Alocando memoria para data, uint16 ocupa 2 bytes, por isso dividi por 2 o tamanho do data
    som.data = new uint16_t[wcid.tam/2];
    if(som.data == nullptr)
    {
        cerr << "Falha na aloca��o de mem�ria para data" << endl;
        return ERR_ALLOC;
    }

    //! Lendo o data Subchunk
    inFile.read((char *) &som, 8);
    inFile.read((char *) som.data, som.tam_data);

    //! Definindo quantidade de caracteres em metadados
    total_caracteres_metadados = wcid.tam - som.tam_data;

    //! Lendo a area de metadados com padrao LIST INFO
    inFile.read((char *) &meta, sizeof(header_METADATA));

    while(1)
    {
        inFile.read((char *) &lista[qtd_metadata], 8);

        if(lista[qtd_metadata].info_id[0] != 'I')
            break;

        lista[qtd_metadata].detalhe = new char[lista[qtd_metadata].tam_info];
        if(lista[qtd_metadata].detalhe == nullptr)
        {
            cerr << "Falha na aloca��o de mem�ria para metadados" << endl;
            return ERR_ALLOC;
        }
        inFile.read(lista[qtd_metadata].detalhe, lista[qtd_metadata].tam_info);
        qtd_metadata++;
    }

    //! Copiando o resto dos metadados para uma variavel(nao tenho certeza do padrao)
    inFile.seekg(inFile.tellg()-8);
    inFile.read(completar_meta , total_caracteres_metadados - meta.tam_meta - 44);

    //! Adaptando o nome para o arquivo de sa�da
    filename = (filename.substr(0,filename.size()-4)) += "_corte.wav";

    //! Abrindo o arquivo de sa�da
    outfile.open(filename, ios::binary);
    if(!outfile)
    {
        cerr << "Arquivo n�o pode ser criado" << endl;
        return ERR_CR_ARQ;
    }

    //! While para escolhas do usuario
    while(escolha_usuario == 1 || escolha_usuario == 2)
    {
        menu_principal();
        cin >> escolha_usuario;
        if(escolha_usuario == 1)
            show_info(wcid, format);

        if(escolha_usuario == 2)
            cortar_audio(format ,som , outfile);
    }

    //! Completando metadados do arquivo de saida

    outfile.write(reinterpret_cast <const char *> (&meta) , 12);
    for(int i = 0 ; i < qtd_metadata ; i++)
    {
        outfile.write(reinterpret_cast <const char *> (&lista[i]) , 8);
        for(int y = 0; y < lista[i].tam_info; y++)
            outfile.write(reinterpret_cast <const char *> (&lista[i].detalhe[y]) , 1);
    }

    for(int i = 0 ; i < total_caracteres_metadados - meta.tam_meta - 44; i++)
        outfile.write(reinterpret_cast <const char *> (&completar_meta[i]) , 1);

    //! desalocando a memoria e fechando arquivos
    inFile.close();
    outfile.close();
    delete som.data;
    for(int i = 0; i < qtd_metadata; i++)
        delete lista[i].detalhe;
    return OK;
}

/************************************************************
* @brief adiciona o sufixo a string se necessario
* @param string contendo nome do arquivo dado pelo usuario
* @retval string com sufixo .txt
************************************************************/
void addsufixo(string &nome_arq)
{
    void inverte(string &str);

    inverte(nome_arq);
    size_t encontrado = nome_arq.find("vaw.");
    inverte(nome_arq);
    if(encontrado != string::npos);
    else nome_arq += ".wav";
}

/************************************************************
* @brief inverte a string
* @param string a ser invertida
************************************************************/
void inverte(string &str) {
    string invertida;
    for(int i = str.size() - 1; i >= 0; i--) invertida += str[i];
    str = invertida;
}

/************************************************************
* @brief printa o menu na tela
************************************************************/
void menu_principal(void)
{
    cout << endl << "         MENU" << endl << "OP��ES : " << endl;
    cout << "digite '1' para abrir o arquivo e mostrar suas informa��es" << endl;
    cout << "digite '2' para recortar o trecho da musica desejado" << endl;
    cout << "digite qualquer outro caracter para encerrar o programa" << endl;
}

/********************************************
* @brief mostrar informacoes do arquivo wav
* @param struct contendo o audio
* @param tamanho do audio
********************************************/
void show_info(wavCHK1_t &descriptor, wavCHK2_t &info_audio)
{
    {
        cout << endl << "Tamanho do arquivo em bytes: " << (descriptor.tam + 8) << endl;
        cout << "tamanho do subchunk: " << info_audio.Subchunk1Size << endl;
        cout << "Formato do audio(1 � PCM): " << info_audio.audio_format <<  endl;
        cout << "Numero de canais(mono = 1)(stereo = 2): " << info_audio.num_chan << endl;
        cout << "quantidade de amostras por segundo: " << info_audio.sample_sec << endl;
        cout << "quantidade de bytes em um segundo de audio:" << info_audio.bytes_sec << endl;
        cout << "numero de bytes por uma amostra:" << info_audio.block_allign << endl;
        cout << "bits por amostra: " << info_audio.bits_per_sample << endl;
    }
}

/********************************************
* @brief recorta a parte do audio desejada
* @param arquivo de base para informa��es
* @param arquivo contendo o audio
* @param arquivo de saida
*********************************************/
void cortar_audio(wavCHK2_t base, wavCHK3_t &audio, ofstream &outfile)
{
    void criar_wav(ofstream &outfile, wavCHK2_t base, uint32_t &tamanho);

    float final_corte;
    float inicio_corte;

    cout << "segundo para o inicio do corte(MAX 3 n�meros depois do ponto): ";
    cin >> inicio_corte;
    cout << "Segundo para o final do corte(MAX 3 n�meros depois do ponto): ";
    cin >> final_corte;

    if(inicio_corte > final_corte)
    {
        int troca = inicio_corte;
        inicio_corte = final_corte;
        final_corte = troca;
    }
    inicio_corte = inicio_corte * base.bytes_sec;
    final_corte = final_corte * base.bytes_sec;
    uint32_t tamanho = (final_corte - inicio_corte)+total_caracteres_metadados;

    criar_wav(outfile, base, tamanho);

    for(int i = (inicio_corte/2); i <  (final_corte/2); i++)
        outfile.write(reinterpret_cast<const char*> (&audio.data[i]), 2);
}

/**********************************************
* @brief cria um arquivo wav
* @param arquivo de saida
* @param arquivo de base para informa��es
* @param tamanho do arquivo a ser criado
**********************************************/
void criar_wav(ofstream &outfile, wavCHK2_t format, uint32_t &tamanho)
{
    outfile << "RIFF";
    outfile.write(reinterpret_cast<const char*> (&tamanho), 4);
    tamanho -= total_caracteres_metadados;
    outfile << "WAVE";
    outfile << "fmt ";
    outfile.write(reinterpret_cast<const char*> (&format.Subchunk1Size), 4);
    outfile.write(reinterpret_cast<const char*> (&format.audio_format), 2);
    outfile.write(reinterpret_cast<const char*> (&format.num_chan), 2);
    outfile.write(reinterpret_cast<const char*> (&format.sample_sec), 4);
    outfile.write(reinterpret_cast<const char*> (&format.bytes_sec), 4);
    outfile.write(reinterpret_cast<const char*> (&format.block_allign), 2);
    outfile.write(reinterpret_cast<const char*> (&format.bits_per_sample), 2);
    outfile << "data";
    outfile.write(reinterpret_cast<const char*> (&tamanho), 4);
}
