#include "main.h"
#include "calc.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <tuple>
#include <limits>
#include <chrono>
#include <ctime>

using namespace std;

int main (){

    const string path = "..\\disks\\disco2.img";

    fstream fat16DISK(path, ios::in | ios::out | ios::binary);
    if (!fat16DISK.is_open()) {
        cout << "Erro ao Abrir o arquivo";
        return 1;
    }

    BootSector bootSector;
    if (!readBootSector(fat16DISK, bootSector)) {
        cout << "Falha ao ler Boot Sector" << endl;
        return 1;
    }
    vector<DirectoryEntry> rootDirectoryEntries;
    if (!readRootDirectory(fat16DISK, bootSector, rootDirectoryEntries)) {
        cout << "Falha ao ler Root Directory" << endl;
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
                printRootDirectory(rootDirectoryEntries);
                break;
            case 2:
                    listFileContent(fat16DISK, bootSector, rootDirectoryEntries);
                break;
            case 3:
                listAttributes(rootDirectoryEntries);
                break;
            case 4:
                renameFile(fat16DISK,bootSector,rootDirectoryEntries);
                break;
            case 5:
                deleteFile(fat16DISK, bootSector, rootDirectoryEntries);
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
bool readBootSector(fstream& disk, BootSector& boot) {
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
    cout << "Tamanho da FAT (setores): " << boot.FATSize << endl;
}

bool readRootDirectory(fstream& disk, const BootSector& boot, vector<DirectoryEntry>& entries) {
    streamoff rootDirectoryOffset = calcRootDirOffset(boot);
    int rootDirectorySize = boot.rootEntries * sizeof(DirectoryEntry);

    disk.seekg(rootDirectoryOffset, ios::beg);

    entries.resize(boot.rootEntries); // Define vector de entries com |x| = limite de entries dado no disco
    disk.read(reinterpret_cast<char*>(entries.data()), rootDirectorySize); // Popula cada entry preenchendo o struct

    return disk.good();
}

void printRootDirectory(const vector<DirectoryEntry>& entries)
{
    auto trim = [](string s) {
        size_t end = s.find_last_not_of(' ');
        if (end != string::npos)
            s.erase(end + 1);
        else
            s.clear();
        return s;
    };

    cout << endl << "=== Arquivos no diretorio raiz ===" << endl;

    for (const auto& entry : entries) {
        if (entry.name[0] == 0x00) break;        // end of list
        if ((uint8_t)entry.name[0] == 0xE5) continue; // deleted

        // skip volume label & subdirectories
        if (!(entry.attr & 0x08) && !(entry.attr & 0x10)) {
            string name = trim(string(entry.name, entry.name + 8));
            string extension = trim(string(entry.name + 8, entry.name + 11));

            cout << name;
            if (!extension.empty()) cout << "." << extension;
            cout << " (" << entry.fileSize << " bytes)" << endl;
        }
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

uint16_t findFile(const vector<DirectoryEntry>& entries, const char* name) {
    uint16_t entryNumber = 0;

    for (const auto& entry : entries) {
        entryNumber++;
        if (entry.name[0] == 0x00) return 0;
        if ((uint8_t)entry.name[0] == 0xE5) continue;
        if ((entry.attr & 0x0F) == 0x0F || (entry.attr & 0x08) || (entry.attr & 0x10)) continue;
        if (memcmp(entry.name, name, 11) == 0) return entryNumber;
    }

    return 0; // not found
}

void listFileContent(fstream& disk, const BootSector& boot, vector<DirectoryEntry>& entries) {
    char name11[11];
    if (!readFat16Name(name11)) return;

    uint16_t filePos = findFile(entries, name11);
    if (filePos == 0 || filePos > entries.size()) {
        cout << "Arquivo nao encontrado\n";
        return;
    }

    DirectoryEntry& entry = entries[filePos - 1];
    if (entry.fileSize == 0) {
        cout << "[Arquivo vazio]\n";
        return;
    }

    uint32_t remaining = entry.fileSize;
    uint16_t cluster   = entry.firstCluster;
    const uint32_t clusterBytes = static_cast<uint32_t>(boot.sectorsPerCluster) * boot.bytesPerSector;

    string out;
    out.reserve(remaining);

    while (remaining > 0 && cluster >= 0x0002 && cluster < 0xFFF8) {
        if (cluster == 0xFFF7 || cluster == 0x0000) { // bad or free => corrupt chain
            cout << "[Corrupcao na cadeia de clusters]" << endl;
            break;
        }

        streamoff coff = calcClusterOffset(boot, cluster);
        disk.seekg(coff, ios::beg);
        if (!disk.good()) { cout << "[Falha no seek]" << endl; break; }

        size_t toRead = min<uint32_t>(remaining, clusterBytes);
        vector<char> buf(toRead);
        disk.read(buf.data(), static_cast<streamsize>(toRead));
        if (!disk.good()) { cout << "[Falha na leitura]" << endl; break; }

        out.append(buf.begin(), buf.end());
        remaining -= static_cast<uint32_t>(toRead);

        if (remaining == 0) break;
        cluster = readFATEntry(disk, boot, cluster);
    }

    // print

    int printable = 0;
    for (unsigned char c : out)
        if (c >= 0x20 || c == '\n' || c == '\r' || c == '\t') ++printable;

    if (!out.empty() && double(printable) / out.size() > 0.7) {
        cout << out << "\n";
    } else {
        for (size_t i = 0; i < out.size(); ++i) {
            printf("%02X", static_cast<unsigned char>(out[i]));
            if ((i + 1) % 16 == 0) printf("\n");
            else if ((i + 1) % 2 == 0) printf(" ");
        }
        if (!out.empty() && out.size() % 16 != 0) printf("\n");
    }
}

void listAttributes(vector<DirectoryEntry> entries) {
    char name[11];
    if (!readFat16Name(name)) {
        return;
    }

    uint16_t filePos = findFile(entries, name);
    if (filePos == 0) {
        cout << "Arquivo nao encontrado" << endl;
        return;
    }
    DirectoryEntry& entry = entries[filePos - 1];
    printFileAttributes(entry);
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

void renameFile(fstream& disk, const BootSector& boot, vector<DirectoryEntry>& entries) {
    char oldName[11];
    if (!readFat16Name(oldName)) return;

    uint16_t filePos = findFile(entries, oldName);
    if (filePos == 0 || filePos > entries.size()) {
        cout << "Arquivo nao encontrado" << endl;
        return;
    }

    cout << "Novo nome - ";
    char newName[11];
    if (!readFat16Name(newName)) return;

    if (findFile(entries, newName) != 0) {
        cout << "Um Arquivo com este nome ja existe" << endl;
        return;
    }

    DirectoryEntry& entry = entries[filePos - 1];
    strncpy(entry.name, newName, sizeof(entry.name));

    streamoff rootOffset = calcRootDirOffset(boot);
    streamoff entryOffset = rootOffset + (filePos - 1) * sizeof(DirectoryEntry);

    disk.seekp(entryOffset, ios::beg);
    disk.write(newName, 11);
    disk.flush();

    cout << "Renomeado com sucesso" << endl;
}

void deleteFile(fstream& disk, const BootSector& boot, vector<DirectoryEntry>& entries) {
    char name[11];
    if (!readFat16Name(name)) return;

    uint16_t filePos = findFile(entries, name);
    if (filePos == 0 || filePos > entries.size()) {
        cout << "Arquivo nao encontrado" << endl;
        return;
    }

    DirectoryEntry& entry = entries[filePos - 1];
    if (entry.firstCluster < 2) {
        cout << "Arquivo sem clusters alocados" << endl;
    } else {
        uint16_t cluster = entry.firstCluster;
        while (cluster >= 0x0002 && cluster < 0xFFF8) {
            uint16_t next = readFATEntry(disk, boot, cluster);

            streamoff fatOffset = calcFATOffset(boot);
            disk.seekp(fatOffset, ios::beg);
            uint16_t zero = 0x0000;
            disk.write(reinterpret_cast<char*>(&zero), sizeof(zero));

            if (next == 0xFFFF || next >= 0xFFF8) break;
            cluster = next;
        }
    }
    entry.name[0] = (char)0xE5;

    streamoff rootOffset = calcRootDirOffset(boot);
    streamoff entryOffset = rootOffset + (filePos - 1) * sizeof(DirectoryEntry);

    disk.seekp(entryOffset, ios::beg);
    disk.write(reinterpret_cast<char*>(&entry), sizeof(DirectoryEntry));
    disk.flush();

    cout << "Arquivo deletado com sucesso" << endl;
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