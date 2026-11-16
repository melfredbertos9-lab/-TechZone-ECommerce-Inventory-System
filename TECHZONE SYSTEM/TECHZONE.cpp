#define NOMINMAX
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#define PICK(win, other) win
#else
#define PICK(win, other) other
#endif
using namespace std;

// ---- Console look & feel (orange "marketplace" theme) and small helpers ----
static const string RST = "\033[0m", BLD = "\033[1m", DIM = "\033[2m", ORG = "\033[38;5;208m",
                    RED = "\033[38;5;196m", GRN = "\033[38;5;40m", YEL = "\033[38;5;220m",
                    GRY = "\033[38;5;245m", WHT = "\033[97m", BGO = "\033[48;5;202m";
static const int W = 86;

static string lowerText(string t) {
    transform(t.begin(), t.end(), t.begin(), [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return t;
}
static string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    return a == string::npos ? "" : s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}
static string padR(string s, int w) {
    if (static_cast<int>(s.size()) > w - 1 && w > 1) s = s.substr(0, w - 1);
    return s + string(max(0, w - static_cast<int>(s.size())), ' ');
}
static string padL(string s, int w) {
    if (static_cast<int>(s.size()) > w) s = s.substr(0, w);
    return string(w - static_cast<int>(s.size()), ' ') + s;
}
static string money(double v) {  // 15990 -> "15,990"
    string s = to_string(llround(v));
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3) s.insert(i, ",");
    return s;
}
static string oneDecimal(double v, const string& suffix) { ostringstream o; o << fixed << setprecision(1) << v << suffix; return o.str(); }
static string soldText(int sold) { return sold < 1000 ? to_string(sold) : oneDecimal(sold / 1000.0, "k"); }
static string ratingText(double r, int sold) { return sold == 0 ? "New" : oneDecimal(r, "*"); }
static void line(char c = '-') { cout << GRY << string(W, c) << RST << "\n"; }
static void header(const string& title) { cout << BGO << BLD << WHT << padR("  TECHZONE  |  " + title, W) << RST << "\n"; }

static void clearScreen() { if (system(PICK("cls", "clear"))) {} }
static void pauseScreen() { string dummy; cout << GRY << "\nPress Enter to continue..." << RST; getline(cin, dummy); }

// ---- Data record + table printing ----
struct Product {
    int id = 0;
    string name, category, brand;
    double price = 0;
    int stock = 0, sold = 0, discount = 0;  // discount = percent off shown on the "Flash Sale" badge
    double rating = 0;
};

static void printProductHeader() {
    cout << GRY << padR("ID", 6) << padR("Product", 27) << padR("Brand", 9) << padL("Price (PHP)", 12) << "  "
         << padR("Off", 6) << padL("Sold", 7) << "  " << padR("Rating", 7) << padL("Stock", 6) << RST << "\n";
    line();
}

static void printProductRow(const Product& p) {
    string stockTxt = p.stock == 0 ? RED + padL("OUT", 6) + RST
                    : p.stock <= 3 ? YEL + padL(to_string(p.stock), 6) + RST : padL(to_string(p.stock), 6);
    string offTxt = p.discount > 0 ? RED + padR("-" + to_string(p.discount) + "%", 6) + RST : padR("", 6);
    cout << GRY << padR(to_string(p.id), 6) << RST << padR(p.name, 27) << DIM << padR(p.brand, 9) << RST
         << ORG << BLD << padL(money(p.price), 12) << RST << "  " << offTxt << padL(soldText(p.sold), 7) << "  "
         << YEL << padR(ratingText(p.rating, p.sold), 7) << RST << stockTxt << "\n";
}

// [Lab 1] Dynamic allocation: self-resizing product catalog
class ProductCatalog {
    Product* items;
    int count = 0, capacity = 8;
    string lastEvent = "none yet";
    void resize(int newCap) {
        Product* bigger = new Product[newCap]();
        for (int i = 0; i < count; i++) bigger[i] = items[i];
        delete[] items;
        lastEvent = "capacity " + to_string(capacity) + " -> " + to_string(newCap);
        items = bigger;
        capacity = newCap;
    }
public:
    ProductCatalog() { items = new Product[capacity](); }
    ~ProductCatalog() { delete[] items; }
    ProductCatalog(const ProductCatalog&) = delete;
    ProductCatalog& operator=(const ProductCatalog&) = delete;
    void add(const Product& p) { if (count == capacity) resize(capacity * 2); items[count++] = p; }  // grow: double
    void removeAt(int idx) {
        for (int i = idx; i < count - 1; i++) items[i] = items[i + 1];
        count--;
        if (capacity > 8 && count <= capacity / 4) resize(capacity / 2);  // shrink: halve
    }
    Product& at(int i) { return items[i]; }
    int size() const { return count; }
    int cap() const { return capacity; }
    const string& event() const { return lastEvent; }
};

// [Lab 6] BST index on product ID  (node -> position in the catalog array)
struct BSTNode {
    int id, index;
    BSTNode *left = nullptr, *right = nullptr;
    BSTNode(int i, int x) : id(i), index(x) {}
};

class ProductIndex {
    BSTNode* root = nullptr;
    int nodes = 0;
    BSTNode* build(const vector<pair<int, int>>& v, int lo, int hi) {
        if (lo > hi) return nullptr;
        int mid = lo + (hi - lo) / 2;  // median first -> balanced tree
        BSTNode* n = new BSTNode(v[mid].first, v[mid].second);
        n->left = build(v, lo, mid - 1);
        n->right = build(v, mid + 1, hi);
        return n;
    }
    void destroy(BSTNode* n) { if (n) { destroy(n->left); destroy(n->right); delete n; } }
    void inOrder(BSTNode* n, vector<int>& out) const { if (n) { inOrder(n->left, out); out.push_back(n->id); inOrder(n->right, out); } }
    int heightOf(BSTNode* n) const { return n ? 1 + max(heightOf(n->left), heightOf(n->right)) : 0; }
public:
    ~ProductIndex() { destroy(root); }
    void rebuild(ProductCatalog& cat) {
        destroy(root);
        vector<pair<int, int>> v;
        for (int i = 0; i < cat.size(); i++) v.push_back({cat.at(i).id, i});
        sort(v.begin(), v.end());
        nodes = static_cast<int>(v.size());
        root = build(v, 0, nodes - 1);
    }
    // Returns catalog index or -1. steps = comparisons, path = IDs visited.
    int search(int id, int& steps, vector<int>* path = nullptr) const {
        steps = 0;
        for (BSTNode* cur = root; cur; cur = id < cur->id ? cur->left : cur->right) {
            steps++;
            if (path) path->push_back(cur->id);
            if (id == cur->id) return cur->index;
        }
        return -1;
    }
    vector<int> ascendingIds() const { vector<int> out; inOrder(root, out); return out; }
    int height() const { return heightOf(root); }
    int size() const { return nodes; }
    int rootId() const { return root ? root->id : -1; }
};

// [Lab 2] Doubly linked list: shopping cart (prev/next navigation)
struct CartNode {
    int productId, qty;
    CartNode *prev = nullptr, *next = nullptr;
    CartNode(int id, int q) : productId(id), qty(q) {}
};

class ShoppingCart {
    CartNode *head = nullptr, *tail = nullptr, *cur = nullptr;
    int count = 0;
public:
    ~ShoppingCart() { clear(); }
    void add(int id, int qty) {
        for (CartNode* n = head; n; n = n->next)
            if (n->productId == id) { n->qty += qty; cur = n; return; }
        CartNode* node = new CartNode(id, qty);
        if (!tail) head = tail = node; else { tail->next = node; node->prev = tail; tail = node; }
        cur = node;
        count++;
    }
    bool next() { if (cur && cur->next) { cur = cur->next; return true; } return false; }
    bool prev() { if (cur && cur->prev) { cur = cur->prev; return true; } return false; }
    void removeCurrent() {
        if (!cur) return;
        CartNode* n = cur;
        if (n->prev) n->prev->next = n->next; else head = n->next;
        if (n->next) n->next->prev = n->prev; else tail = n->prev;
        cur = n->next ? n->next : n->prev;
        delete n;
        count--;
    }
    void removeProduct(int id) {
        for (CartNode* n = head; n;) { CartNode* nx = n->next; if (n->productId == id) { cur = n; removeCurrent(); } n = nx; }
    }
    void clear() { while (head) { CartNode* t = head; head = head->next; delete t; } tail = cur = nullptr; count = 0; }
    int qtyOf(int id) const { for (CartNode* n = head; n; n = n->next) if (n->productId == id) return n->qty; return 0; }
    int positionOfCurrent() const { int pos = 0; for (CartNode* n = head; n; n = n->next) if (++pos, n == cur) return pos; return 0; }
    CartNode* first() const { return head; }
    CartNode* current() const { return cur; }
    int size() const { return count; }
};

// [Lab 2] Singly linked list: customer wishlist
struct WishNode {
    int productId;
    WishNode* next = nullptr;
    explicit WishNode(int id) : productId(id) {}
};

class Wishlist {
    WishNode* head = nullptr;
    int count = 0;
public:
    ~Wishlist() { clear(); }
    bool contains(int id) const { for (WishNode* n = head; n; n = n->next) if (n->productId == id) return true; return false; }
    bool add(int id) {
        if (contains(id)) return false;
        WishNode* node = new WishNode(id);
        if (!head) head = node; else { WishNode* t = head; while (t->next) t = t->next; t->next = node; }
        count++;
        return true;
    }
    bool remove(int id) {
        WishNode *c = head, *p = nullptr;
        while (c && c->productId != id) { p = c; c = c->next; }
        if (!c) return false;
        if (p) p->next = c->next; else head = c->next;
        delete c;
        count--;
        return true;
    }
    void clear() { while (head) { WishNode* t = head; head = head->next; delete t; } count = 0; }
    WishNode* first() const { return head; }
    int size() const { return count; }
};

// [Lab 3] Circular linked list: continuous brand banner showcase
struct BannerNode {
    string brand, tagline;
    BannerNode* next = nullptr;
    BannerNode(const string& b, const string& t) : brand(b), tagline(t) {}
};

class BannerCarousel {
    BannerNode *tail = nullptr, *cur = nullptr;  // tail->next is always the head (circular)
    int count = 0;
public:
    ~BannerCarousel() {
        if (!tail) return;
        BannerNode* n = tail->next;
        tail->next = nullptr;  // break the ring, then free from the head
        while (n) { BannerNode* t = n; n = n->next; delete t; }
    }
    void add(const string& brand, const string& tagline) {
        BannerNode* n = new BannerNode(brand, tagline);
        if (!tail) { tail = n; n->next = n; cur = n; } else { n->next = tail->next; tail->next = n; tail = n; }
        count++;
    }
    void advance() { if (cur) cur = cur->next; }
    void reset() { if (tail) cur = tail->next; }
    BannerNode* current() const { return cur; }
    BannerNode* head() const { return tail ? tail->next : nullptr; }
    int size() const { return count; }
};

// [Lab 4] Stack (LIFO): admin Undo / Redo
enum ActionType { ACT_ADD, ACT_DELETE, ACT_UPDATE };
struct Action { ActionType type = ACT_ADD; Product before, after; string desc; };
struct StackNode {
    Action data;
    StackNode* next;
    StackNode(const Action& a, StackNode* n) : data(a), next(n) {}
};

class ActionStack {
    StackNode* topNode = nullptr;
    int count = 0;
public:
    ~ActionStack() { clear(); }
    void push(const Action& a) { topNode = new StackNode(a, topNode); count++; }
    bool pop(Action& out) {
        if (!topNode) return false;
        StackNode* t = topNode;
        out = t->data;
        topNode = t->next;
        delete t;
        count--;
        return true;
    }
    void clear() { Action tmp; while (pop(tmp)) {} }
    StackNode* top() const { return topNode; }
    int size() const { return count; }
};

// [Lab 5] Queue (FIFO): customer checkout & order processing line
struct OrderLine { int productId; string name; int qty; double unitPrice; };
struct Order { int orderId = 0; string customer; vector<OrderLine> lines; double total = 0; };
struct OrderNode {
    Order data;
    OrderNode* next = nullptr;
    explicit OrderNode(const Order& o) : data(o) {}
};

class OrderQueue {
    OrderNode *front = nullptr, *rear = nullptr;
    int count = 0;
public:
    ~OrderQueue() { Order tmp; while (dequeue(tmp)) {} }
    void enqueue(const Order& o) {
        OrderNode* n = new OrderNode(o);
        if (!rear) front = rear = n; else { rear->next = n; rear = n; }
        count++;
    }
    bool dequeue(Order& out) {
        if (!front) return false;
        OrderNode* t = front;
        out = t->data;
        front = front->next;
        if (!front) rear = nullptr;
        delete t;
        count--;
        return true;
    }
    OrderNode* first() const { return front; }
    int size() const { return count; }
};

// [Lab 8] Sorting algorithms (work on arrays of Product pointers = "views")
enum SortKey { BY_PRICE, BY_STOCK, BY_BRAND };

template <class T> static int cmp3(const T& x, const T& y) { return x < y ? -1 : (y < x ? 1 : 0); }

static bool precedes(const Product* a, const Product* b, SortKey k, bool asc) {
    int c = k == BY_PRICE ? cmp3(a->price, b->price) : k == BY_STOCK ? cmp3(a->stock, b->stock)
          : cmp3(lowerText(a->brand) + "\n" + lowerText(a->name), lowerText(b->brand) + "\n" + lowerText(b->name));  // brand, then name
    return asc ? c < 0 : c > 0;
}

static void insertionSort(Product** a, int n, SortKey k, bool asc, long& cmp) {
    for (int i = 1; i < n; i++) {
        Product* key = a[i];
        int j = i - 1;
        for (; j >= 0; j--) { cmp++; if (!precedes(key, a[j], k, asc)) break; a[j + 1] = a[j]; }
        a[j + 1] = key;
    }
}

static void quickSort(Product** a, int lo, int hi, SortKey k, bool asc, long& cmp) {
    if (lo >= hi) return;
    swap(a[lo + (hi - lo) / 2], a[hi]);  // middle element as pivot
    Product* pivot = a[hi];
    int i = lo;
    for (int j = lo; j < hi; j++) {
        cmp++;
        if (precedes(a[j], pivot, k, asc)) swap(a[i++], a[j]);
    }
    swap(a[i], a[hi]);
    quickSort(a, lo, i - 1, k, asc, cmp);
    quickSort(a, i + 1, hi, k, asc, cmp);
}

// [Lab 7] Binary search on a price-sorted view
//   upper=false: first index with price >= v   |   upper=true: first index with price > v
static int boundPrice(Product** a, int n, double v, bool upper, int& steps) {
    int lo = 0, hi = n;
    while (lo < hi) {
        steps++;
        int mid = lo + (hi - lo) / 2;
        if (upper ? a[mid]->price <= v : a[mid]->price < v) lo = mid + 1; else hi = mid;
    }
    return lo;
}

// The store
class TechZone {
    static const int BRAND_COUNT = 10;
    const string brands[BRAND_COUNT] = {"Samsung", "Apple", "ASUS", "Lenovo", "Xiaomi", "OPPO", "Huawei", "HP", "Dell", "Acer"};
    const string taglines[BRAND_COUNT] = {
        "Galaxy of possibilities - flagship phones and tablets", "Think different - iPhone and MacBook lineup",
        "In search of incredible - Vivobook, Zenbook and ROG", "Smarter technology for all - IdeaPad, ThinkPad and Legion",
        "Innovation for everyone - Redmi and Xiaomi flagships", "Inspiration ahead - Reno and Find series",
        "Make it possible - Nova, Pura and Mate", "Keep reinventing - Pavilion, Envy, Spectre and Omen",
        "The power to do more - Inspiron, XPS and Alienware", "Explore beyond limits - Aspire, Swift, Nitro and Predator"};

    ProductCatalog catalog;  // Lab 1
    ProductIndex index;      // Lab 6
    ShoppingCart cart;       // Lab 2 (doubly)
    Wishlist wishlist;       // Lab 2 (singly)
    BannerCarousel banners;  // Lab 3
    ActionStack undoStack, redoStack;  // Lab 4
    OrderQueue orders;       // Lab 5
    bool exitRequested = false;
    int nextOrderId = 1001, processedOrders = 0;
    double revenue = 0;

    // ---------------- Input helpers ----------------
    int readInt(const string& prompt) {
        int v;
        while (true) {
            cout << prompt;
            if (cin >> v) { cin.ignore(1000, '\n'); return v; }
            if (cin.eof()) exit(0);  // input closed: stop instead of looping forever
            cout << RED << "Invalid input. Please enter a whole number.\n" << RST;
            cin.clear(); cin.ignore(1000, '\n');
        }
    }
    double readDouble(const string& prompt) {
        double v;
        while (true) {
            cout << prompt;
            if (cin >> v && v >= 0) { cin.ignore(1000, '\n'); return v; }
            if (cin.eof()) exit(0);
            cout << RED << "Invalid input. Please enter a non-negative number.\n" << RST;
            cin.clear(); cin.ignore(1000, '\n');
        }
    }
    string readLine(const string& prompt) {
        string v;
        cout << prompt;
        if (!getline(cin, v) && cin.eof()) exit(0);
        return v;
    }
    string readCmd(const string& prompt) { return lowerText(trim(readLine(prompt))); }
    void msg(const string& color, const string& text) { cout << color << text << RST << "\n"; }  // coloured one-liner

    // ---------------- Brand helpers ----------------
    int brandIndex(const string& b) const {
        for (int i = 0; i < BRAND_COUNT; i++) if (lowerText(b) == lowerText(brands[i])) return i;
        return -1;
    }
    string brandList() const { string s; for (int i = 0; i < BRAND_COUNT; i++) s += (i ? "/" : "") + brands[i]; return s; }

    // ---------------- Catalog access (via BST index) ----------------
    Product* getById(int id) { int steps = 0, idx = index.search(id, steps); return idx < 0 ? nullptr : &catalog.at(idx); }
    vector<Product*> collect(bool (*keep)(const Product&, const string&), const string& arg = "") {
        vector<Product*> v;
        for (int i = 0; i < catalog.size(); i++) if (keep(catalog.at(i), arg)) v.push_back(&catalog.at(i));
        return v;
    }
    vector<Product*> everything() { return collect([](const Product&, const string&) { return true; }); }
    vector<Product*> ofBrand(const string& b) { return collect([](const Product& p, const string& x) { return p.brand == x; }, b); }
    bool addProductRecord(const Product& p) {
        if (getById(p.id)) return false;
        catalog.add(p);
        index.rebuild(catalog);
        return true;
    }
    bool removeProductById(int id) {
        int steps = 0, idx = index.search(id, steps);
        if (idx < 0) return false;
        catalog.removeAt(idx);
        index.rebuild(catalog);
        cart.removeProduct(id);
        wishlist.remove(id);
        return true;
    }

    // ---------------- Shared UI pieces ----------------
    void statusBar() {
        cout << ORG << " Cart (" << cart.size() << ")   Wishlist (" << wishlist.size() << ")   Orders in queue (" << orders.size()
             << ")" << RST << GRY << "   |   Memory: " << catalog.size() << "/" << catalog.cap() << RST << "\n";
        line();
    }
    void flashSale() {  // top 3 in-stock products by discount
        vector<Product*> v;
        for (int i = 0; i < catalog.size(); i++) if (catalog.at(i).stock > 0) v.push_back(&catalog.at(i));
        stable_sort(v.begin(), v.end(), [](Product* a, Product* b) { return a->discount > b->discount; });
        cout << BLD << RED << " FLASH SALE " << RST;
        for (size_t i = 0; i < v.size() && i < 3; i++)
            cout << GRY << " | " << RST << v[i]->name << " " << RED << "-" << v[i]->discount << "%" << RST << " " << ORG << "P"
                 << money(v[i]->price) << RST;
        cout << "\n";
        line();
    }
    void menuItem(int n, const string& text, const string& lab = "") {
        cout << " " << ORG << BLD << padL(to_string(n), 2) << "." << RST << " " << padR(text, 46) << GRY << lab << RST << "\n";
    }

    // ---------------- Product page & list browser ----------------
    bool addToCart(Product* p, int qty) {
        if (qty <= 0) { msg(RED, "Quantity must be at least 1."); return false; }
        if (cart.qtyOf(p->id) + qty > p->stock) {
            msg(RED, "Not enough stock. Available: " + to_string(p->stock) + ", already in your cart: " + to_string(cart.qtyOf(p->id)) + ".");
            return false;
        }
        cart.add(p->id, qty);
        msg(GRN, "Added " + to_string(qty) + " x " + p->name + " to your cart.");
        return true;
    }
    void productPage(Product* p) {
        while (true) {
            clearScreen();
            header("Product Details");
            cout << "\n " << BLD << p->name << RST << "\n " << BGO << WHT << " Official Mall " << RST << " " << p->brand << GRY
                 << "   Category: " << p->category << RST << "\n\n " << YEL << ratingText(p->rating, p->sold) << RST << GRY << "  |  "
                 << RST << soldText(p->sold) << " sold   " << GRY << "|  Product ID " << p->id << RST << "\n\n   " << ORG << BLD
                 << "PHP " << money(p->price) << RST;
            if (p->discount > 0)
                cout << "   " << GRY << "PHP " << money(p->price / (1.0 - p->discount / 100.0)) << RST << "  " << RED << "-"
                     << p->discount << "% OFF" << RST;
            cout << "\n\n   Shipping : " << GRN << "Free Shipping" << RST << " - Metro Manila\n   Stock    : "
                 << (p->stock == 0 ? RED + "Sold out" + RST : (p->stock <= 3 ? YEL : GRN) + to_string(p->stock) + " left" + RST) << "\n\n";
            line();
            cout << " 1. Add to Cart    2. Add to Wishlist    0. Back\n";
            int c = readInt(" Choose: ");
            if (c == 0) return;
            if (c == 1) {
                if (p->stock == 0) msg(RED, "This product is sold out."); else addToCart(p, readInt(" Quantity: "));
                pauseScreen();
            } else if (c == 2) {
                if (wishlist.add(p->id)) msg(GRN, "Added to your wishlist."); else msg(YEL, "Already in your wishlist.");
                pauseScreen();
            }
        }
    }

    // Paged result viewer used by every listing. Type a product ID to open it.
    void browse(const string& title, const string& note, const vector<Product*>& list) {
        const int PAGE = 10, n = static_cast<int>(list.size());
        int page = 0, pages = max(1, (n + PAGE - 1) / PAGE);
        while (true) {
            clearScreen();
            header(title);
            if (!note.empty()) cout << GRY << " " << note << RST << "\n";
            line();
            if (n == 0) { msg(YEL, " No products found."); pauseScreen(); return; }
            printProductHeader();
            for (int i = page * PAGE; i < min(n, (page + 1) * PAGE); i++) printProductRow(*list[i]);
            line();
            cout << (pages > 1 ? " [N]ext  [P]revious  [Q]uit" : " [Q]uit") << "   or type a Product ID to open it\n";
            string cmd = readCmd(" > ");
            if (cmd == "q" || cmd == "0") return;
            if (cmd == "n" || cmd.empty()) { if (page + 1 < pages) page++; else if (pages == 1) return; }
            else if (cmd == "p") { if (page > 0) page--; }
            else if (cmd.size() < 9 && all_of(cmd.begin(), cmd.end(), [](unsigned char c) { return isdigit(c) != 0; })) {
                Product* p = getById(stoi(cmd));
                if (p) productPage(p); else { msg(RED, " Product ID not found."); pauseScreen(); }
            }
        }
    }

    // ================= 1. View complete catalog (Lab 1) =================
    void viewCatalog() {
        browse("Complete Catalog", "Dynamic array: " + to_string(catalog.size()) + " items in " + to_string(catalog.cap()) +
               " slots | last resize: " + catalog.event(), everything());
    }

    // ================= 2. Filter by official brand =================
    void filterByBrand() {
        clearScreen();
        header("Shop by Official Brand");
        for (int i = 0; i < BRAND_COUNT; i++)
            cout << " " << ORG << BLD << padL(to_string(i + 1), 2) << "." << RST << " " << padR(brands[i], 12) << GRY
                 << ofBrand(brands[i]).size() << " products" << RST << "\n";
        cout << "  0. Back\n";
        int c = readInt("\n Choose a brand: ");
        if (c < 1 || c > BRAND_COUNT) return;
        browse(brands[c - 1] + " Official Store", "Filtered by brand (linear scan of " + to_string(catalog.size()) + " items)", ofBrand(brands[c - 1]));
    }

    // ================= 3. Keyword search - linear search (Lab 7) =================
    void searchKeyword() {
        clearScreen();
        header("Search  -  Linear Keyword Match");
        string q = lowerText(trim(readLine(" Search name, brand or category: ")));
        if (q.empty()) return;
        vector<Product*> res = collect([](const Product& p, const string& k) {
            return lowerText(p.name).find(k) != string::npos || lowerText(p.brand).find(k) != string::npos ||
                   lowerText(p.category).find(k) != string::npos; }, q);
        browse("Search results for \"" + q + "\"", "Linear search: " + to_string(catalog.size()) + " products checked, " +
               to_string(res.size()) + " matched", res);
    }

    // ================= 4. Price range - binary search (Lab 7 + 8) =================
    void filterPriceRange() {
        clearScreen();
        header("Filter by Price Range  -  Binary Search");
        double lo = readDouble(" Minimum price (PHP): "), hi = readDouble(" Maximum price (PHP): ");
        if (hi < lo) swap(lo, hi);
        vector<Product*> byPrice = everything();
        int n = static_cast<int>(byPrice.size()), steps = 0;
        long sortCmp = 0;
        quickSort(byPrice.data(), 0, n - 1, BY_PRICE, true, sortCmp);  // binary search needs sorted data
        int first = boundPrice(byPrice.data(), n, lo, false, steps), last = boundPrice(byPrice.data(), n, hi, true, steps);
        browse("Price PHP " + money(lo) + " - " + money(hi), "Binary search: " + to_string(steps) + " steps (linear scan would need " +
               to_string(n) + ") | cheapest first", vector<Product*>(byPrice.begin() + first, byPrice.begin() + max(first, last)));
    }

    // ================= 5. Cart & wishlist (Lab 2) =================
    double cartTotal() {
        double t = 0;
        for (CartNode* n = cart.first(); n; n = n->next) if (Product* p = getById(n->productId)) t += p->price * n->qty;
        return t;
    }
    void printCartCard() {
        CartNode* c = cart.current();
        Product* p = getById(c->productId);
        cout << " Item " << BLD << cart.positionOfCurrent() << RST << " of " << cart.size() << "    " << GRY
             << (c->prev ? "<< prev" : "|< first") << "   " << (c->next ? "next >>" : "last >|") << RST << "\n\n";
        if (!p) { msg(RED, " This product is no longer available."); return; }
        cout << "   " << BLD << p->name << RST << "  " << GRY << "(" << p->brand << ")" << RST << "\n   PHP " << money(p->price) << " x "
             << c->qty << " = " << ORG << BLD << "PHP " << money(p->price * c->qty) << RST << "\n";
    }
    void cartScreen() {
        while (true) {
            clearScreen();
            header("My Shopping Cart");
            if (cart.size() == 0) cout << "\n" << YEL << " Your cart is empty. Browse the catalog to add items!\n" << RST;
            else {
                cout << "\n";
                printCartCard();
                cout << "\n " << GRY << "Cart total: " << RST << ORG << BLD << "PHP " << money(cartTotal()) << RST << "\n";
            }
            line();
            cout << " [N]ext item  [P]revious item  [V]iew all  [R]emove this item\n [C]lear cart  [W]ishlist  [B] Check out  [0] Back\n";
            string cmd = readCmd(" > ");
            if (cmd == "0") return;
            if (cmd == "n" && !cart.next()) { msg(YEL, " That is the last item."); pauseScreen(); }
            else if (cmd == "p" && !cart.prev()) { msg(YEL, " That is the first item."); pauseScreen(); }
            else if (cmd == "v") {
                clearScreen();
                header("Cart  -  All Items");
                if (cart.size() == 0) msg(YEL, " Cart is empty.");
                for (CartNode* n = cart.first(); n; n = n->next)
                    if (Product* p = getById(n->productId))
                        cout << " " << padR(p->name, 28) << padL(to_string(n->qty) + " x", 6) << ORG << padL("PHP " + money(p->price * n->qty), 18) << RST << "\n";
                if (cart.size() > 0) { line(); cout << ORG << BLD << padL("TOTAL  PHP " + money(cartTotal()), 52) << RST << "\n"; }
                pauseScreen();
            } else if (cmd == "r" && cart.size() > 0) { cart.removeCurrent(); msg(GRN, " Item removed."); pauseScreen(); }
            else if (cmd == "c") { cart.clear(); msg(GRN, " Cart cleared."); pauseScreen(); }
            else if (cmd == "w") wishlistScreen();
            else if (cmd == "b") { placeOrder(); pauseScreen(); }
        }
    }
    void wishlistScreen() {
        while (true) {
            clearScreen();
            header("My Wishlist  (singly linked list)");
            if (wishlist.size() == 0) cout << "\n" << YEL << " Your wishlist is empty. Open a product and choose \"Add to Wishlist\".\n" << RST;
            else {
                cout << "\n";
                printProductHeader();
                for (WishNode* n = wishlist.first(); n; n = n->next) if (Product* p = getById(n->productId)) printProductRow(*p);
            }
            line();
            cout << " [M]ove item to cart   [R]emove item   [0] Back\n";
            string cmd = readCmd(" > ");
            if (cmd == "0") return;
            if ((cmd == "m" || cmd == "r") && wishlist.size() > 0) {
                int id = readInt(" Product ID: ");
                Product* p = getById(id);
                if (!p || !wishlist.contains(id)) msg(RED, " That product is not in your wishlist.");
                else if (cmd == "r") { wishlist.remove(id); msg(GRN, " Removed from wishlist."); }
                else if (p->stock == 0) msg(RED, " Sold out.");
                else if (addToCart(p, 1)) wishlist.remove(id);
                pauseScreen();
            }
        }
    }

    // ================= 6. Checkout queue (Lab 5) =================
    bool placeOrder() {
        if (cart.size() == 0) { msg(YEL, " Your cart is empty."); return false; }
        Order o;
        for (CartNode* n = cart.first(); n; n = n->next) {
            Product* p = getById(n->productId);
            if (!p) continue;
            if (n->qty > p->stock) {
                msg(RED, " Cannot check out: only " + to_string(p->stock) + " of " + p->name + " left (you have " + to_string(n->qty) + " in the cart).");
                return false;
            }
            o.lines.push_back({p->id, p->name, n->qty, p->price});
            o.total += p->price * n->qty;
        }
        if (o.lines.empty()) { msg(YEL, " Nothing available to check out."); return false; }
        o.orderId = nextOrderId++;
        o.customer = "You";
        orders.enqueue(o);
        cart.clear();
        cout << GRN << BLD << " Order #" << o.orderId << " placed!" << RST << "\n Total: " << ORG << "PHP " << money(o.total) << RST
             << "   Your position in the checkout line: " << orders.size() << "\n";
        return true;
    }
    void viewQueue() {
        clearScreen();
        header("Order Processing Line  (FIFO)");
        if (!orders.first()) cout << "\n" << YEL << " The checkout queue is empty.\n" << RST;
        else cout << "\n";
        int pos = 0;
        for (OrderNode* n = orders.first(); n; n = n->next) {
            pos++;
            cout << (pos == 1 ? ORG + BLD + " NEXT " + RST : GRY + padL(to_string(pos) + ".", 6) + RST) << " Order #" << n->data.orderId
                 << "  " << padR(n->data.customer, 6) << padL(to_string(n->data.lines.size()) + " item(s)", 11) << ORG
                 << padL("PHP " + money(n->data.total), 16) << RST << "\n";
        }
        pauseScreen();
    }
    void processNextOrder() {
        clearScreen();
        header("Process Next Order");
        Order o;
        if (!orders.dequeue(o)) { cout << "\n" << YEL << " The checkout queue is empty.\n" << RST; pauseScreen(); return; }
        cout << "\n Order #" << o.orderId << " (" << o.customer << ")\n\n";
        for (const OrderLine& l : o.lines) {  // check every line first so a failed order deducts nothing
            Product* p = getById(l.productId);
            if (!p || p->stock < l.qty) {
                cout << RED << BLD << " FAILED" << RST << " - " << l.name
                     << (p ? " has insufficient stock (" + to_string(p->stock) + " left)" : " no longer exists") << ".\n";
                msg(GRY, " The order was cancelled; no stock was deducted.");
                pauseScreen();
                return;
            }
        }
        for (const OrderLine& l : o.lines) {
            Product* p = getById(l.productId);
            p->stock -= l.qty;
            p->sold += l.qty;
            cout << "  " << padR(l.name, 28) << padL(to_string(l.qty) + " x", 6) << ORG << padL("PHP " + money(l.unitPrice * l.qty), 16) << RST << "\n";
        }
        line();
        cout << GRN << BLD << " Order #" << o.orderId << " completed." << RST << "  Total " << ORG << BLD << "PHP " << money(o.total) << RST << "\n";
        processedOrders++;
        revenue += o.total;
        pauseScreen();
    }
    void checkoutMenu() {
        while (true) {
            clearScreen();
            header("Checkout & Order Queue  (FIFO)");
            cout << GRY << " Orders waiting: " << orders.size() << "   Processed: " << processedOrders << "   Revenue: PHP " << money(revenue) << RST << "\n";
            line();
            cout << " 1. Check out my cart  (cart total PHP " << money(cartTotal()) << ")\n"
                 << " 2. View order processing line\n 3. Seller Centre: process next order  (dequeue front)\n 0. Back\n";
            int c = readInt(" Choose: ");
            if (c == 0) return;
            if (c == 1) { placeOrder(); pauseScreen(); } else if (c == 2) viewQueue(); else if (c == 3) processNextOrder();
        }
    }

    // ================= 7. Seller Centre: admin actions + undo / redo (Lab 4) =================
    void recordAction(ActionType t, const Product& before, const Product& after, const string& desc) {
        Action a;
        a.type = t; a.before = before; a.after = after; a.desc = desc;
        undoStack.push(a);
        redoStack.clear();  // a new action invalidates the redo history
    }
    void applyHistory(bool undo) {  // undo pops undoStack -> redoStack; redo does the reverse
        ActionStack& from = undo ? undoStack : redoStack;
        ActionStack& to = undo ? redoStack : undoStack;
        const string verb = undo ? "undo" : "redo";
        Action a;
        if (!from.pop(a)) { msg(YEL, " Nothing to " + verb + "."); return; }
        bool ok;
        if (a.type == ACT_UPDATE) {
            Product* p = getById(a.after.id);
            ok = p != nullptr;
            if (p) *p = undo ? a.before : a.after;
        } else {  // ADD/DELETE are mirror images: undoing an add removes it, redoing it re-adds it
            Product& t = a.type == ACT_ADD ? a.after : a.before;
            ok = ((a.type == ACT_ADD) == undo) ? removeProductById(t.id) : addProductRecord(t);
        }
        if (ok) { to.push(a); cout << GRN << " Done (" << verb << "): " << RST << a.desc << "\n"; }
        else msg(RED, " Could not " + verb + ": " + a.desc);
    }
    void adminAddProduct() {
        Product p;
        p.id = readInt(" New product ID: ");
        if (p.id <= 0 || getById(p.id)) { msg(RED, " ID must be positive and not already used."); return; }
        p.name = trim(readLine(" Name: "));
        p.category = trim(readLine(" Category: "));
        int b = brandIndex(trim(readLine(" Brand (" + brandList() + "): ")));
        if (p.name.empty() || b < 0) { msg(RED, " Name is required and brand must be one of the listed brands."); return; }
        p.brand = brands[b];
        p.price = readDouble(" Price (PHP): ");
        p.stock = max(0, readInt(" Stock: "));
        addProductRecord(p);
        recordAction(ACT_ADD, Product(), p, "Added " + p.name);
        msg(GRN, " Product added.");
    }
    void adminUpdateProduct() {
        Product* p = getById(readInt(" Product ID: "));
        if (!p) { msg(RED, " Product not found."); return; }
        Product before = *p;
        double price = readDouble(" New price (PHP): ");
        int stock = readInt(" New stock: ");
        if (stock < 0) { msg(RED, " Stock cannot be negative."); return; }
        p->price = price;
        p->stock = stock;
        recordAction(ACT_UPDATE, before, *p, "Updated " + before.name);
        msg(GRN, " Product updated.");
    }
    void adminDeleteProduct() {
        int id = readInt(" Product ID to delete: ");
        Product* p = getById(id);
        if (!p) { msg(RED, " Product not found."); return; }
        Product before = *p;  // copy first: removal invalidates p
        removeProductById(id);
        recordAction(ACT_DELETE, before, Product(), "Deleted " + before.name);
        msg(GRN, " Product deleted.");
    }
    void sellerCentre() {
        while (true) {
            clearScreen();
            header("Seller Centre  -  Inventory & Undo / Redo  (LIFO stacks)");
            cout << " Undo stack: " << BLD << undoStack.size() << RST << GRY << "  top -> " << RST
                 << (undoStack.top() ? undoStack.top()->data.desc : "(empty)") << "\n Redo stack: " << BLD << redoStack.size() << RST << GRY
                 << "  top -> " << RST << (redoStack.top() ? redoStack.top()->data.desc : "(empty)") << "\n";
            line();
            cout << " 1. Undo last action        4. Add a product\n 2. Redo                     5. Update price / stock\n"
                 << " 3. View undo history       6. Delete a product\n 0. Back\n";
            int c = readInt(" Choose: ");
            if (c == 0) return;
            if (c == 1 || c == 2) applyHistory(c == 1);
            else if (c == 3) {
                int i = 0;
                cout << "\n";
                for (StackNode* n = undoStack.top(); n; n = n->next) cout << " " << ++i << ". " << n->data.desc << "\n";
                if (i == 0) msg(YEL, " No history yet.");
            } else if (c == 4) adminAddProduct();
            else if (c == 5) adminUpdateProduct();
            else if (c == 6) adminDeleteProduct();
            else continue;
            pauseScreen();
        }
    }

    // ================= 8. Brand banner showcase (Lab 3) =================
    void showcase() {
        banners.reset();
        while (true) {
            clearScreen();
            header("Brand Showcase  -  Featured Banners");
            BannerNode *cur = banners.current(), *n = banners.head();
            cout << " ";
            for (int i = 0; i < banners.size(); i++, n = n->next) cout << (n == cur ? BGO + WHT + " " + n->brand + " " + RST : GRY + n->brand + RST) << " > ";
            cout << ORG << "(back to " << banners.head()->brand << ")" << RST << "\n\n";
            vector<Product*> mine = ofBrand(cur->brand);
            Product *top = nullptr, *deal = nullptr;
            for (Product* p : mine) {
                if (!top || p->sold > top->sold) top = p;
                if (!deal || p->discount > deal->discount) deal = p;
            }
            cout << ORG << string(W, '=') << RST << "\n   " << BLD << ORG << cur->brand << " OFFICIAL STORE" << RST << "\n   " << cur->tagline
                 << "\n" << ORG << string(W, '=') << RST << "\n   Products in store : " << mine.size() << "\n";
            if (top) cout << "   Best seller       : " << top->name << " (" << soldText(top->sold) << " sold)\n";
            if (deal && deal->discount > 0)
                cout << "   Top deal          : " << deal->name << " " << RED << "-" << deal->discount << "%" << RST << " now PHP " << money(deal->price) << "\n";
            line();
            cout << " [Enter] Next banner   [A] Auto-cycle one full lap   [S] Shop this brand   [0] Back\n";
            string cmd = readCmd(" > ");
            if (cmd == "0" || cmd == "q") return;
            if (cmd.empty() || cmd == "n") banners.advance();
            else if (cmd == "a") {
                cout << "\n";
                BannerNode* t = banners.current();
                for (int i = 0; i <= banners.size(); i++, t = t->next)
                    cout << "  " << GRY << (i == 0 ? "start " : "  -> ") << RST << padR(t->brand, 10) << GRY << t->tagline << RST << "\n";
                msg(ORG, "  The list is circular: after the last banner it wraps back to the first.");
                pauseScreen();
            } else if (cmd == "s") browse(cur->brand + " Official Store", "Shop from the banner", mine);
        }
    }

    // ================= 9. Sort inventory display (Lab 8) =================
    void sortInventory() {
        clearScreen();
        header("Sort Inventory Display");
        cout << " Sort by:  1. Price   2. Stock   3. Brand (then name)\n";
        int k = readInt(" Choose: ");
        if (k < 1 || k > 3) return;
        cout << " Order:    1. Ascending   2. Descending\n";
        int d = readInt(" Choose: ");
        if (d < 1 || d > 2) return;
        cout << " Algorithm: 1. Insertion Sort   2. Quick Sort   3. Run both and compare\n";
        int a = readInt(" Choose: ");
        if (a < 1 || a > 3) return;
        SortKey key = k == 1 ? BY_PRICE : (k == 2 ? BY_STOCK : BY_BRAND);
        vector<Product*> view = everything(), copy = view;  // each algorithm starts from its own unsorted copy
        int n = static_cast<int>(view.size());
        long cmpIns = 0, cmpQuick = 0;
        if (a != 2) insertionSort(a == 1 ? view.data() : copy.data(), n, key, d == 1, cmpIns);
        if (a != 1) quickSort(view.data(), 0, n - 1, key, d == 1, cmpQuick);
        string note = a == 1 ? "Insertion Sort: " + to_string(cmpIns) + " comparisons"
                    : a == 2 ? "Quick Sort: " + to_string(cmpQuick) + " comparisons"
                             : "Insertion: " + to_string(cmpIns) + " comparisons  vs  Quick: " + to_string(cmpQuick) + " comparisons";
        browse("Sorted Inventory", note, view);
    }

    // ================= 10. Find product by ID - BST (Lab 6) =================
    void bstMenu() {
        while (true) {
            clearScreen();
            header("Product Index  -  Binary Search Tree");
            cout << GRY << " Nodes: " << index.size() << "   Height: " << index.height() << "   Root ID: " << index.rootId()
                 << "   (worst case " << index.height() << " steps instead of " << index.size() << ")" << RST << "\n";
            line();
            cout << " 1. Find a product by ID\n 2. Show all IDs (in-order traversal)\n 0. Back\n";
            int c = readInt(" Choose: ");
            if (c == 0) return;
            if (c == 1) {
                int id = readInt(" Product ID: "), steps = 0;
                vector<int> path;
                int idx = index.search(id, steps, &path);
                cout << "\n " << GRY << "Search path: " << RST;
                for (size_t i = 0; i < path.size(); i++) cout << (i ? " -> " : "") << path[i];
                cout << "\n " << GRY << "Steps: " << steps << RST << "\n\n";
                if (idx < 0) msg(RED, " Product ID " + to_string(id) + " is not in the index.");
                else { printProductHeader(); printProductRow(catalog.at(idx)); }
                pauseScreen();
            } else if (c == 2) {
                vector<int> ids = index.ascendingIds();
                cout << "\n";
                for (size_t i = 0; i < ids.size(); i++) cout << padL(to_string(ids[i]), 6) << ((i + 1) % 12 == 0 ? "\n" : "");
                cout << "\n";
                pauseScreen();
            }
        }
    }

    // ================= Sample data =================
    void loadSampleProducts() {
        Product samples[] = {
            {101, "iPhone 11", "Smartphone", "Apple", 15990.00, 12}, {102, "iPhone 12", "Smartphone", "Apple", 19990.00, 10}, {103, "iPhone 13", "Smartphone", "Apple", 24990.00, 9},
            {104, "iPhone 14", "Smartphone", "Apple", 29990.00, 8}, {105, "iPhone 15", "Smartphone", "Apple", 34990.00, 7}, {106, "iPhone 16", "Smartphone", "Apple", 42990.00, 6},
            {107, "iPhone 17", "Smartphone", "Apple", 49990.00, 5}, {108, "iPhone 17 Pro", "Smartphone", "Apple", 64990.00, 4}, {109, "iPhone 17 Pro Max", "Smartphone", "Apple", 74990.00, 3},
            {110, "MacBook Air M3", "Laptop", "Apple", 59990.00, 5}, {111, "MacBook Pro M4", "Laptop", "Apple", 99990.00, 2}, {201, "Galaxy S23", "Smartphone", "Samsung", 24990.00, 10},
            {202, "Galaxy S24", "Smartphone", "Samsung", 32990.00, 9}, {203, "Galaxy S25", "Smartphone", "Samsung", 39990.00, 7}, {204, "Galaxy S26", "Smartphone", "Samsung", 45990.00, 5},
            {205, "Galaxy A15", "Smartphone", "Samsung", 7990.00, 18}, {206, "Galaxy A35", "Smartphone", "Samsung", 16990.00, 12}, {207, "Galaxy Tab S10", "Tablet", "Samsung", 32990.00, 6},
            {301, "Vivobook 15", "Laptop", "ASUS", 28990.00, 10}, {302, "Vivobook 16", "Laptop", "ASUS", 32990.00, 8}, {303, "Zenbook 14", "Laptop", "ASUS", 54990.00, 4},
            {304, "ROG Zephyrus G14", "Gaming Laptop", "ASUS", 89990.00, 3}, {305, "ROG Strix G16", "Gaming Laptop", "ASUS", 79990.00, 4}, {401, "IdeaPad Slim 3", "Laptop", "Lenovo", 24990.00, 11},
            {402, "IdeaPad Slim 5", "Laptop", "Lenovo", 34990.00, 7}, {403, "ThinkPad E14", "Business Laptop", "Lenovo", 39990.00, 6}, {404, "ThinkPad T14", "Business Laptop", "Lenovo", 64990.00, 3},
            {405, "Legion 5", "Gaming Laptop", "Lenovo", 69990.00, 4}, {501, "Redmi Note 13", "Smartphone", "Xiaomi", 11990.00, 15}, {502, "Redmi Note 14", "Smartphone", "Xiaomi", 13990.00, 14},
            {503, "Xiaomi 14T", "Smartphone", "Xiaomi", 29990.00, 8}, {504, "Xiaomi 15", "Smartphone", "Xiaomi", 44990.00, 6}, {505, "Xiaomi Pad 6", "Tablet", "Xiaomi", 17990.00, 7},
            {601, "OPPO A58", "Smartphone", "OPPO", 8990.00, 16}, {602, "OPPO Reno 11", "Smartphone", "OPPO", 21990.00, 10}, {603, "OPPO Reno 12", "Smartphone", "OPPO", 24990.00, 9},
            {604, "OPPO Find X7", "Smartphone", "OPPO", 49990.00, 4}, {605, "OPPO Find N3 Flip", "Foldable Phone", "OPPO", 54990.00, 3}, {701, "Huawei Nova 12", "Smartphone", "Huawei", 17990.00, 9},
            {702, "Huawei Pura 70", "Smartphone", "Huawei", 44990.00, 5}, {703, "Huawei Mate 60 Pro", "Smartphone", "Huawei", 59990.00, 4}, {704, "MateBook D 16", "Laptop", "Huawei", 42990.00, 6},
            {705, "MatePad Pro 13", "Tablet", "Huawei", 39990.00, 5}, {801, "HP Pavilion 15", "Laptop", "HP", 31990.00, 10}, {802, "HP Pavilion Plus 14", "Laptop", "HP", 45990.00, 7},
            {803, "HP Envy x360", "Laptop", "HP", 52990.00, 5}, {804, "HP Spectre x360", "Premium Laptop", "HP", 89990.00, 3}, {805, "HP Omen 16", "Gaming Laptop", "HP", 74990.00, 4},
            {901, "Dell Inspiron 15", "Laptop", "Dell", 29990.00, 11}, {902, "Dell Inspiron 16", "Laptop", "Dell", 36990.00, 8}, {903, "Dell XPS 13", "Premium Laptop", "Dell", 79990.00, 4},
            {904, "Dell XPS 15", "Premium Laptop", "Dell", 99990.00, 3}, {905, "Dell Alienware m16", "Gaming Laptop", "Dell", 109990.00, 2}, {1001, "Acer Aspire 3", "Laptop", "Acer", 22990.00, 12},
            {1002, "Acer Aspire 5", "Laptop", "Acer", 29990.00, 9}, {1003, "Acer Swift Go 14", "Laptop", "Acer", 44990.00, 6}, {1004, "Acer Nitro V 15", "Gaming Laptop", "Acer", 49990.00, 7},
            {1005, "Acer Predator Helios 16", "Gaming Laptop", "Acer", 94990.00, 3}};
        for (Product& p : samples) {
            // Demo "marketplace" stats derived from the ID so they stay the same every run.
            p.sold = (p.id * 37) % 900 + 40;
            p.rating = 4.0 + ((p.id * 7) % 10) / 10.0;
            p.discount = (p.id % 5) * 5;
            catalog.add(p);  // grows 8 -> 16 -> 32 -> 64 automatically
        }
        index.rebuild(catalog);
    }

    // ================= Home page (opens automatically, no login) =================
    void homePage() {
        typedef void (TechZone::*Screen)();
        static const Screen screens[10] = {&TechZone::viewCatalog, &TechZone::filterByBrand, &TechZone::searchKeyword, &TechZone::filterPriceRange,
            &TechZone::cartScreen, &TechZone::checkoutMenu, &TechZone::sellerCentre, &TechZone::showcase, &TechZone::sortInventory, &TechZone::bstMenu};
        static const char* MENU[10][2] = {
            {"View Complete Catalog", "Dynamic array"}, {"Shop by Official Brand", "10 partner brands"},
            {"Search Products by Keyword", "Linear search"}, {"Filter Products by Price Range", "Binary search"},
            {"Shopping Cart & Wishlist", "Doubly / singly linked list"}, {"Checkout & Order Queue", "FIFO queue"},
            {"Seller Centre: Inventory, Undo / Redo", "LIFO stack"}, {"Brand Banner Showcase", "Circular list"},
            {"Sort Inventory Display (Price/Stock/Brand)", "Insertion / quick sort"}, {"Find Product by ID", "BST index"}};
        while (!exitRequested) {
            clearScreen();
            header("Official Mall");
            statusBar();
            flashSale();
            for (int i = 0; i < 10; i++) menuItem(i + 1, MENU[i][0], MENU[i][1]);
            menuItem(0, "Exit");
            line();
            int c = readInt(" Enter your choice: ");
            if (c == 0) exitRequested = true;
            else if (c >= 1 && c <= 10) (this->*screens[c - 1])();
            else { msg(RED, " Invalid menu option."); pauseScreen(); }
        }
    }

public:
    TechZone() {
        for (int i = 0; i < BRAND_COUNT; i++) banners.add(brands[i], taglines[i]);
        loadSampleProducts();
    }
    void run() {
        homePage();
        clearScreen();
        cout << ORG << BLD << "Thank you for shopping at TechZone. Goodbye!\n" << RST;
    }
};

int main() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) SetConsoleMode(h, mode | 0x0004);  // enable ANSI colors
#endif
    TechZone store;
    store.run();
    return 0;
}
