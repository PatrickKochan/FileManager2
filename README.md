﻿# FileManager2

Universidade Federal de Santa Catarina - UFSC<br>
Centro de Ciências, Tecnologias e Saúde<br>
Departamento de Computação<br>
Curso de Engenharia da Computação<br><br>

Alunos:<br>
Matheus André Soares - 15103102<br>
Patrick Davila Kochan - 15102827<br><br>

Enunciado do Trabalho:<br>
    O sistema deve permitir que arquivos locais sejam acessados por usuarios remotos simultaneamente.<br>
    As operacoes permitidas pelo sistema devem incluir:<br>
    criar (sub)diretorio<br>
    remover (sub)diretorio<br>
    entrar em (sub)diretorio<br>
    mostrar conteudo do diretorio<br>
    criar arquivo<br>
    remover arquivo<br>
    escrever um sequencia de caracteres em um arquivo<br>
    mostrar conteudo do arquivo<br><br>

    Etapa 1:
        Desenvolver a estrutura de acesso do servidor de arquivos.
        Ele devera sera acessado via socket TCP. Cada conexao devera ser gerida por uma thread.
        Condicoes de corrida deverao ser tratadas por meio de semaforos ou mutexes.
        Nesta etapa voce nao precisa implementar as operacoes sobre arquivos listadas acima.
        Ao inves disso, use as operacoes diretamente do sistema de arquivos do seu sistema operacional.
        Recomenda-se que o servidor imprima mensagens na tela para demonstrar o funcionamento ao professor.
    
    Etapa 2
        Estenda o trabalho desenvolvendo a estrutura de alocacao e organizacao de arquivos do servidor de arquivos.
        Mais precisamente, agora a aplicação servidor utilizara um arquivo em disco que simulara o sistema de arquivos EXT3.
        Dentro deste arquivo havera no minimo tres regioes: uma regiao para o controle de espaço livre, uma região para armazenar inodes
        e uma regiao para armazenar dados de arquivos e diretorios. 
        As operacoes devem ser implementadas usando o arquivo que simula o sistema de arquivos. Por exemplo, assuma que o arquivo que 
        simula o sistema de arquivos se chama sa.bin e que um usuario quer criar um arquivo chamado teste.txt no servidor contendo os
        caracteres "abcd" (4 bytes). Entao, dentro de sa.bin o servidor deve encontrar um inode livre, atribuir o nome teste.txt ao
        inode e associar um bloco livre de 4 bytes para conter "abcd". 
        Ainda dentro de sa.bin, grava-se "abcd" no bloco livre associado e se deduz espaço utilizado para guardar "abcd" do espaço 
        livre.

A thread no servidor deve receber uma solicitação do cliente e então repassá-la a um novo processo via pipe nomeado e este deverá executar a ação solicitada e devolver ao servidor uma mensagem de status, informando se a solicitação foi realizada com sucesso ou com erro.

    Observacoes:
        Nao e necessario autenticacao dos usuarios.
        Nao e necessario criar um aplicativo cliente. Voce pode usar o aplicativo netcat disponivel para Linux e Windows.
     
Anotacoes da equipe:<br>
    - Algumas funcionalidades do sistema foram implementadas de maneira simplicada tendo em vista que nao eram o foco do trabalho.<br>
    - A funcao cat mostra apenas a primeira linha do arquivo (funcao fgets)<br>
    - Sao iniciados dois clientes. O primeiro deles inicia a insercao de um comando mkdir para criar uma pasta
no diretorio atual do servidor (diretorio A). O outro cliente tambem inicia a insercao de um comando, um cd,
e realiza a mudanca de diretorio confirmando a operacao (diretorio B). O servidor troca de diretorio. O primeiro 
cliente confirma enfim a operacao mkdir. O resultado sera a criacao da pasta no diretorio B. Deveriamos adaptar o 
mutex para resolver essa situacao.<br>
    - Temos um servidor aberto e dois clientes. Quantas threads e quantos processos temos?<br>
        - 2 threads (clientes) e 1 processo (servidor)?<br><br>
    - As mensagens printadas pelas funcoes "system" nao sao precisas. Ex.: se dermos um "cd" para uma pasta inexistente, a mensagem de sucesso sera emitida, pois o processo de checagem do retorno aumentaria a complexidade da implementacao.<br><br>
