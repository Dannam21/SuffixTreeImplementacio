#include <iostream>
#include <map>
#include <string>
using namespace std;

string text;

struct Node {
    map<char, Node *> children;
    Node *suffixLink;
    int start;
    int *end;

    Node(int start, int *end)
        : suffixLink(nullptr),
          start(start),
          end(end) {}
};

Node *root          = nullptr;
Node *lastNewNode   = nullptr;
Node *activeNode    = nullptr;
int   activeEdge    = -1;
int   activeLength  = 0;
int   remainingSuffixCount = 0;
int   leafEnd       = -1;
int  *rootEnd       = nullptr;

Node *newNode(int start, int *end) { return new Node(start, end); }

int edgeLength(Node *n)            { return *(n->end) - n->start + 1; }

bool walkDown(Node *currNode)
{
    if (activeLength >= edgeLength(currNode)) {
        activeEdge  += edgeLength(currNode);
        activeLength -= edgeLength(currNode);
        activeNode   = currNode;
        return true;
    }
    return false;
}

void extendSuffixTree(int pos)
{
    leafEnd = pos;
    remainingSuffixCount++;
    lastNewNode = nullptr;

    while (remainingSuffixCount > 0) {
        if (activeLength == 0) activeEdge = pos;

        char currentChar = text[activeEdge];
        if (activeNode->children.find(currentChar) == activeNode->children.end()) {
            // Caso 1: no existe arista – se crea hoja
            activeNode->children[currentChar] = newNode(pos, &leafEnd);

            if (lastNewNode != nullptr) {
                lastNewNode->suffixLink = activeNode;
                lastNewNode = nullptr;
            }
        } else {
            Node *nextNode = activeNode->children[currentChar];
            if (walkDown(nextNode)) continue;

            if (text[nextNode->start + activeLength] == text[pos]) {
                // Caso 2: ya coincide; aumentamos activeLength
                if (lastNewNode != nullptr) {
                    lastNewNode->suffixLink = activeNode;
                    lastNewNode = nullptr;
                }
                activeLength++;
                break;
            }
            // Caso 3: hay que dividir arista
            int *splitEnd   = new int(nextNode->start + activeLength - 1);
            Node *split     = newNode(nextNode->start, splitEnd);

            activeNode->children[currentChar]         = split;
            split->children[text[pos]]                = newNode(pos, &leafEnd);
            nextNode->start += activeLength;
            split->children[text[nextNode->start]]    = nextNode;

            if (lastNewNode != nullptr) lastNewNode->suffixLink = split;
            lastNewNode = split;
        }

        // Siguiente sufijo
        remainingSuffixCount--;
        if (activeNode == root && activeLength > 0) {
            activeLength--;
            activeEdge = pos - remainingSuffixCount + 1;
        } else if (activeNode != root) {
            activeNode = activeNode->suffixLink;
        }
    }
}

void buildSuffixTree()
{
    rootEnd = new int(-1);
    root    = newNode(-1, rootEnd);
    root->suffixLink = root;
    activeNode       = root;

    for (int i = 0; i < static_cast<int>(text.size()); ++i)
        extendSuffixTree(i);
}

void freeTree(Node *n)
{
    if (!n) return;
    for (auto &p : n->children)
        freeTree(p.second);
    delete n;
}

bool searchSubstring(const string &pattern)
{
    Node *current = root;
    int   idx     = 0;
    while (idx < static_cast<int>(pattern.size())) {
        char c = pattern[idx];
        if (current->children.find(c) == current->children.end()) return false;

        Node *next = current->children[c];
        int  len   = edgeLength(next);

        for (int k = 0; k < len && idx < static_cast<int>(pattern.size()); ++k, ++idx) {
            if (text[next->start + k] != pattern[idx]) return false;
        }
        current = next;
    }
    return true;
}

void insertString(const string &str)
{
    for (char ch : str) {
        text += ch;
        extendSuffixTree(text.size() - 1);
    }
}

void deleteSubstring(int start, int length)
{
    if (start < 0 || start + length > static_cast<int>(text.size()) || length <= 0) return;

    text.erase(start, length);

    freeTree(root);
    root = nullptr; lastNewNode = nullptr; activeNode = nullptr;
    activeEdge = -1; activeLength = 0; remainingSuffixCount = 0;
    leafEnd = -1;    rootEnd = nullptr;

    buildSuffixTree();
}

void printSuffixes(Node *n, string current)
{
  if (n->start != -1)
    current += text.substr(n->start, *(n->end) - n->start + 1);
    if (n->children.empty()) {
      cout << current << '\n';
      return;
    }
  for (auto &p : n->children) printSuffixes(p.second, current);
}

int main()
{
  text = "banana$";
  buildSuffixTree();

  cout << "Sufijos representados inicialmente:\n";
  printSuffixes(root, "");

  //busqueda inicial
  cout << "\nBuscar \"ana\": " << (searchSubstring("ana") ? "ENCONTRADO" : "NO") << '\n';

  //insercion de "s$"
  insertString("E$");

  //para imprimir sufijos tras la insercion
  cout << "\nSufijos despues de insertar \"E$\":\n";
  printSuffixes(root, "");

  //busqueda de "anas"
  cout << "\nBuscar \"anaE\": " << (searchSubstring("anaE") ? "ENCONTRADO" : "NO") << '\n';
    
  cout << "\nBuscar \"ana$E\": " << (searchSubstring("ana$E") ? "ENCONTRADO" : "NO") << '\n';


  int delStart = 2, delLen = 3;         
  cout << "\nSe eliminara el substring \""
       << text.substr(delStart, delLen)
       << "\" (posiciones " << delStart << ".."
       << delStart + delLen - 1 << ")\n";

  deleteSubstring(delStart, delLen);

  cout << "\nTexto tras deleteSubstring(2,3): " << text << '\n';
  cout << "¿Aun esta \"ana\"?"
       << (searchSubstring("ana") ? "SÍ" : "NO") << '\n';

  cout << "\nSufijos finales:\n";
  printSuffixes(root, "");

  return 0;
}
