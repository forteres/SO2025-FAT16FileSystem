#include "main.h"

#include <fstream>
#include <iostream>
#include <vector>

using namespace  std;

int main (){

    const string path = "..\\disks\\disco2.img";

    ifstream fat16DISK(path,ios::binary);
    if (!fat16DISK.is_open()) {
        cout << "Erro ao Abrir o arquivo";
        return 1;
    }

    BootSector bootSector;
    if (!readBootSector(fat16DISK, bootSector)) {
        cout << "Falha ao ler Boot Sector.\n";
        return 1;
    }

    int option;
    while (option != 0) {
        evokeMenu();
        cin >> option;
        switch (option) {
            case 1:
                listRootDirectory(fat16DISK,bootSector);
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                printBootInfo(bootSector);
                break;
            case 0:
                fat16DISK.close();
                return 0;
            default:
                cout << "Digite uma opcao valida" << endl;
        }
    }

    fat16DISK.close();
    return 0;
}

// functions
bool readBootSector(ifstream& disk, BootSector& boot) {
    disk.seekg(0, ios::beg);
    disk.read(reinterpret_cast<char*>(&boot), sizeof(BootSector));
    return !disk.fail();
}

void printBootInfo(const BootSector& boot) {
    cout << "=== Boot Sector Info ===" << endl;
    cout << "OEM Name: " << boot.OEMName<< endl;
    cout << "Bytes por setor: " << boot.bytesPerSector << endl;
    cout << "Setores por cluster: " << (int)boot.sectorsPerCluster << endl;
    cout << "Setores reservados: " << boot.reservedSectors << endl;
    cout << "Num. FATs: " << (int)boot.numFATs << endl;
    cout << "Entradas raiz: " << boot.rootEntries << endl;
    // cout << "Small Number of Sectors: " << boot.totalSectors16 << endl;
    // cout << "Media Descriptor: " << (int)boot.mediaDescriptor << endl;
    // cout << "Large Number of Sectors: " << boot.totalSectors32 << endl;
    cout << "Tamanho da FAT (setores): " << boot.FATSize << endl;
}

void listRootDirectory(ifstream& disk, const BootSector& boot) {
    streamoff rootDirectoryOffset = (boot.reservedSectors + boot.numFATs * boot.FATSize) * boot.bytesPerSector;
    int rootDirectorySize = boot.rootEntries * sizeof(DirectoryEntry);

    disk.seekg(rootDirectoryOffset, ios::beg); // Move o ponteiro até o inicio do diretorio root

    vector<DirectoryEntry> entries(boot.rootEntries); // Define vector de entries com |x| = limite de entries dado no disco
    disk.read(reinterpret_cast<char*>(entries.data()), rootDirectorySize); // Popula cada entry preenchendo o struct

    cout << endl << "=== Arquivos no diretorio raiz ===" << endl;
    for (const auto& entry : entries) {
        if (entry.name[0] == 0x00) break;        // fim da lista
        if ((uint8_t)entry.name[0] == 0xE5) continue; // deletado

        if (!(entry.attr & 0x08) && !(entry.attr & 0x10)) { // ignora volume label e subdiretorio
            string name(entry.name, entry.name + 8);
            string extension(entry.name + 8, entry.name + 11);
            name.erase(name.find_last_not_of(' ') + 1);
            extension.erase(extension.find_last_not_of(' ') + 1);

            cout << name;
            if (!extension.empty()) cout << "." << extension;
            cout << " (" << entry.fileSize << " bytes)\n";
        }
    }
}

void evokeMenu() {
    cout << endl << "Menu" << endl;
    cout << "1 - Listar o conteudo do disco" << endl;
    cout << "2 - Listar o conteudo de um arquivo" << endl;
    cout << "3 - Exibir os atributos de um arquivo" << endl;
    cout << "4 - Renomear um arquivo" << endl;
    cout << "5 - Apagar/remover um arquivo" << endl;
    cout << "6 - Inserir/criar um novo arquivo	" << endl;
    cout << "7 - Ler Boot Sector" << endl;
    cout << "0 - Sair" << endl;
}