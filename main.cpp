#include "main.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <tuple>
#include <limits>

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

    int option = -1;
    while (option != 0) {
        evokeMenu();
        cin >> option;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            option = -1;
            continue;
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');    
        }
        switch (option) {
            case 1:
                listRootDirectory(fat16DISK,bootSector);
                break;
            case 2:
                listFileContent(fat16DISK, bootSector);
                break;
            case 3:
                listAttributes(fat16DISK,bootSector);
                break;
            case 4:
                renameFile(path, bootSector);
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
    if (!disk.is_open()) return false;
    disk.seekg(0, ios::beg);
    disk.read(reinterpret_cast<char*>(&boot), sizeof(BootSector));
    return disk.good();
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
    auto trim = [](string s) {
        size_t end = s.find_last_not_of(' ');
        if (end != string::npos)
            s.erase(end + 1);
        else
            s.clear();
        return s;
    };

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
            string name = trim(string(entry.name, entry.name + 8));
            string extension = trim(string(entry.name + 8, entry.name + 11));

            cout << name;
            if (!extension.empty()) cout << "." << extension;
            cout << " (" << entry.fileSize << " bytes)\n";
        }
    }
}

uint64_t calcRootDirOffset(const BootSector& boot) {
    return uint64_t(boot.reservedSectors + boot.numFATs * boot.FATSize) * boot.bytesPerSector;
}

uint64_t calcFirstDataSector(const BootSector& boot) {
    return calcRootDirOffset(boot) + boot.rootEntries * sizeof(DirectoryEntry);
}

uint64_t calcClusterOffset(const BootSector& boot, uint16_t cluster) {
    return calcFirstDataSector(boot) + (cluster - 2) * boot.sectorsPerCluster * boot.bytesPerSector;
}

uint64_t calcFATOffset(const BootSector& boot) {
    return boot.reservedSectors * boot.bytesPerSector;
}

uint16_t readFATEntry(ifstream& disk, const BootSector& boot, uint16_t cluster) {
    uint64_t fatOffset = calcFATOffset(boot) + cluster * 2; // FAT16 usa 2 bytes por entrada
    disk.seekg(fatOffset, ios::beg);
    uint16_t nextCluster;
    disk.read(reinterpret_cast<char*>(&nextCluster), sizeof(nextCluster));
    return nextCluster;
}

void listFileContent(ifstream& disk, const BootSector& boot) {
    char name[11];
    if (!readFat16Name(name)) return;

    unsigned int filePos = findFile(disk, boot, name);
    if (filePos == 0) {
        cout << "Arquivo nao encontrado\n";
        return;
    }

    uint64_t rootOffset = calcRootDirOffset(boot);
    disk.seekg(rootOffset + uint64_t(filePos - 1) * sizeof(DirectoryEntry), ios::beg);
    DirectoryEntry entry;
    disk.read(reinterpret_cast<char*>(&entry), sizeof(DirectoryEntry));

    if (entry.fileSize == 0) {
        cout << "[Arquivo vazio]\n";
        return;
    }

    uint32_t remaining = entry.fileSize;
    uint16_t cluster = entry.firstCluster;
    string out;
    uint32_t clusterBytes = uint32_t(boot.sectorsPerCluster) * boot.bytesPerSector;

    while (remaining > 0 && cluster >= 0x0002 && cluster < 0xFFF8) {
        uint64_t coff = calcClusterOffset(boot, cluster);
        disk.seekg(coff, ios::beg);
        uint32_t toRead = (remaining < clusterBytes) ? remaining : clusterBytes;
        vector<char> buf(toRead);
        disk.read(buf.data(), toRead);
        out.append(buf.begin(), buf.end());
        remaining -= toRead;
        if (remaining == 0) break;
        cluster = readFATEntry(disk, boot, cluster);
    }

    int printable = 0;
    for (unsigned char c : out) if (c >= 0x20 || c == '\n' || c == '\r' || c == '\t') printable++;
    if (out.size() > 0 && double(printable) / out.size() > 0.7) {
        cout << out << "\n";
    } else {
        for (size_t i = 0; i < out.size(); ++i) {
            printf("%02X", (unsigned char)out[i]);
            if ((i+1)%16==0) printf("\n"); else if ((i+1)%2==0) printf(" ");
        }
        printf("\n");
    }
}


bool readFat16Name(char name[11]) {
    string input;
    //cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
    cout << "Digite o nome do arquivo (ex: ARQUIVO.TXT): ";
    
    getline(cin, input);

    input.erase(0, input.find_first_not_of(" \t"));
    input.erase(input.find_last_not_of(" \t") + 1);

    transform(input.begin(), input.end(), input.begin(), ::toupper);

    size_t dotPos = input.find('.');
    string nome = (dotPos == string::npos) ? input : input.substr(0, dotPos);
    string ext  = (dotPos == string::npos) ? ""    : input.substr(dotPos + 1);

    if (nome.empty() || nome.length() > 8 || ext.length() > 3) {
        cout << "Erro: formato invalido (use o padrao 8.3)" << endl;
        return false;
    }

    memset(name, ' ', 11);
    memcpy(name, nome.c_str(), nome.length());
    memcpy(name + 8, ext.c_str(), ext.length());

    return true;
}

unsigned int findFile(ifstream& disk, const BootSector& boot, const char* name) {
    streamoff rootDirectoryOffset = (boot.reservedSectors + boot.numFATs * boot.FATSize) * boot.bytesPerSector;
    int rootDirectorySize = boot.rootEntries * sizeof(DirectoryEntry);

    disk.seekg(rootDirectoryOffset, ios::beg);

    vector<DirectoryEntry> entries(boot.rootEntries);
    disk.read(reinterpret_cast<char*>(entries.data()), rootDirectorySize);

    unsigned int entryNumber = 0;
    for (const auto& entry : entries) {
        entryNumber++;
        if (entry.name[0] == 0x00) {
            entryNumber = 0;
            break;
        }
        if ((uint8_t)entry.name[0] == 0xE5 || (entry.attr & 0x08) || (entry.attr & 0x10)) continue;
        if (memcmp(entry.name,name,11) == 0) break;
    }
    return entryNumber;
}

void printFileAttributes(const DirectoryEntry& entry) {
    auto decodeDate = [](uint16_t date) {
        int day = date & 0x1F;             // bits 0-4
        int month = (date >> 5) & 0x0F;    // bits 5-8
        int year = ((date >> 9) & 0x7F) + 1980; // bits 9-15
        return make_tuple(year, month, day);
    };

    auto decodeTime = [](uint16_t time) {
        int second = (time & 0x1F) * 2;         // bits 0-4
        int minute = (time >> 5) & 0x3F;        // bits 5-10
        int hour = (time >> 11) & 0x1F;         // bits 11-15
        return make_tuple(hour, minute, second);
    };

    auto [cYear, cMonth, cDay] = decodeDate(entry.createDate);
    auto [cHour, cMinute, cSecond] = decodeTime(entry.createTime);
    cout << "Criado em: " << cYear << "/" << cMonth << "/" << cDay
              << " " << cHour << ":" << cMinute << ":" << cSecond << endl;

    auto [mYear, mMonth, mDay] = decodeDate(entry.lastWriteDate);
    auto [mHour, mMinute, mSecond] = decodeTime(entry.lastWriteTime);
    cout << "Ultima modificacao: " << mYear << "/" << mMonth << "/" << mDay
              << " " << mHour << ":" << mMinute << ":" << mSecond << endl;

    cout << "Atributos: ";
    if (entry.attr & 0x01) cout << "[Somente leitura] ";
    if (entry.attr & 0x02) cout << "[Oculto] ";
    if (entry.attr & 0x04) cout << "[Sistema] ";
    if ((entry.attr & 0x3F) == 0) cout << "[Arquivo normal] ";
    cout << endl;
}

void listAttributes(ifstream& disk, const BootSector& boot) {
    char name[11];
    if (!readFat16Name(name)) {
        return;
    }

    unsigned int filePosition = findFile(disk, boot, name);
    if (filePosition == 0) {
        cout << "Arquivo nao encontrado" << endl;
        return;
    }
    streamoff rootDirectoryOffset = (boot.reservedSectors + boot.numFATs * boot.FATSize) * boot.bytesPerSector;
    disk.seekg(rootDirectoryOffset + (filePosition -1) * sizeof(DirectoryEntry), ios::beg);

    DirectoryEntry entry;
    disk.read(reinterpret_cast<char*>(&entry), sizeof(DirectoryEntry));
    printFileAttributes(entry);
}

void renameFile(const string& imagePath, const BootSector& boot) {
    // abrir em read/write
    fstream img(imagePath, ios::in | ios::out | ios::binary);
    if (!img.is_open()) {
        cout << "Erro ao abrir imagem para renomear\n";
        return;
    }
    ifstream disk(imagePath, ios::binary);
    if (!disk.is_open()) {
        cout << "Erro ao abrir imagem para leitura\n";
        img.close();
        return;
    }

    char oldName[11];
    cout << "Nome atual do arquivo:\n";
    if (!readFat16Name(oldName)) { disk.close(); img.close(); return; }

    unsigned int filePos = findFile(disk, boot, oldName);
    if (filePos == 0) {
        cout << "Arquivo nao encontrado\n";
        disk.close(); img.close(); return;
    }

    char newName[11];
    cout << "Novo nome (8.3): \n";
    if (!readFat16Name(newName)) { disk.close(); img.close(); return; }

    unsigned int exists = findFile(disk, boot, newName);
    if (exists != 0) {
        cout << "Erro: ja existe arquivo com esse nome\n";
        disk.close(); img.close(); return;
    }

    uint64_t rootOffset = calcRootDirOffset(boot);
    uint64_t entryOffset = rootOffset + uint64_t(filePos - 1) * sizeof(DirectoryEntry);

    img.seekp(entryOffset, ios::beg);
    img.write(newName, 11);
    img.flush();

    cout << "Renomeado com sucesso.\n";
    disk.close();
    img.close();
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