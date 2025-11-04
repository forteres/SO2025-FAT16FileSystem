# Especificações do Trabalho

## Implementação de Funções de Manipulação de Sistema de Arquivos FAT16
### Objetivo:
Neste trabalho vocês deverão desenvolver um programa em uma linguagem de programação de sua escolha para manipular um sistema de arquivos FAT16. O objetivo principal é implementar operações básicas para gerenciamento de arquivos, possibilitando, por exemplo, a criação, leitura, escrita e remoção de arquivos. Observação: por questões de simplicidade, não estaremos considerando a criação e manipulação de subdiretórios.
### Requisitos:
Implementação de Operações Fundamentais: O programa deve ser capaz de realizar as seguintes operações básicas sobre o sistema de arquivos FAT16:
* Listar o conteúdo do disco: 
  * Exibir em uma lista os nomes dos arquivos (e seus respectivos tamanhos) existentes no diretório raiz.
  * Listar o conteúdo de um arquivo: mostrar (pode ser na tela) o conteúdo de um arquivo do diretório raiz.
  * Exibir os atributos de um arquivo: mostrar data/hora da criação/última modificação e os seguintes atributos: se é somente leitura; se é oculto; se é arquivo de sistema
* Renomear um arquivo: trocar o nome de um arquivo existente
* Inserir/criar um novo arquivo: permitir que se armazene no diretório raiz um novo arquivo externo.
* Apagar/remover um arquivo: apagar um arquivo do diretório raiz.
  
Todas as operações devem ser realizadas sobre um arquivo que contenha uma imagem de disco formatado como FAT16 (um exemplo de arquivo está disponibilizado no AVA Univali). O programa deve ser capaz de ler essa imagem, interpretar sua estrutura e realizar as operações de acordo com as especificações do sistema de arquivos FAT16. Observação: a solução dada deverá funcionar para qualquer arquivo contendo imagem de disco no formato FAT16, sem considerar a existência de subdiretórios.

### Avaliação:
A avaliação do trabalho será baseada nos seguintes critérios:
* Número de Operações Implementadas: Será avaliado o número de operações sobre arquivos que foram implementadas, atribuindo-se os seguintes pesos as realizadas:
  * Listar o conteúdo do disco		10%
  * Listar o conteúdo de um arquivo 	10%
  * Exibir os atributos de um arquivo	10%
  * Renomear um arquivo		10%
  * Apagar/remover um arquivo		20%
  * Inserir/criar um novo arquivo	40%	 
  
Cada uma das operações será avaliada de acordo com a sua corretude: As operações devem ser implementadas de forma correta, seguindo as especificações do sistema de arquivos FAT16.

# Objetivos:

* Listar o conteúdo do disco		10% (X)
* Listar o conteúdo de um arquivo 	10% (X)
* Exibir os atributos de um arquivo	10% (X)
* Renomear um arquivo		10%         (X)
* Apagar/remover um arquivo		20%     (X)
* Inserir/criar um novo arquivo	40%     ()	 