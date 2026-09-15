# Provided GUI - מדריך שימוש

במסגרת הפרויקט מסופק לכם רכיב GUI מוכן להצגת משחק השחמט.

**אין צורך לממש GUI ואין צורך לשנות את הקבצים שבתיקיית `ProvidedGUI`.**

ה-GUI אחראי רק על:

- הצגת לוח השחמט והכלים.
- קבלת בחירת משבצת מקור ויעד מהמשתמש.
- הצגת שמות השחקנים מעל ומתחת ללוח.
- הצגת השחקן שתורו לשחק.
- הצגת הודעת מצב כגון `Check`.
- בחירת כלי במקרה של Promotion.
- כפתור `Resign` שמדווח לקוד על בקשת כניעה, ואפשרות להציג חלון אישור `Confirm / No`.
- כפתור `Offer Draw`.
- קבלת או דחיית הצעת תיקו.
- הצגת סיכום בסיום המשחק.

ה-GUI **אינו מממש את חוקי השחמט**, אינו מנהל משחקים, אינו מבצע תקשורת רשת ואינו יודע אם מהלך מסוים חוקי.

---

# 1. מבנה הקבצים

בהגשה שלכם ניתן לשמור את הרכיב בצורה הבאה:

```text
YourProject/
│
├── client/
│   ├── main.cpp
│   ├── ...
│
├── server/
│   ├── main.cpp
│   ├── ...
│
└── ProvidedGUI/
    ├── ChessFrontend.hpp
    ├── ChessFrontend.cpp
    └── assets/
        ├── white_king.png
        ├── white_queen.png
        ├── ...
        └── black_pawn.png
```

יש להשאיר את תיקיית `assets` בתוך `ProvidedGUI`.

---

# 2. הוספת ה-GUI ל-Client

ה-GUI מיועד להיות חלק מתוכנית ה-Client שלכם.

בקובץ שבו אתם משתמשים ב-GUI:

```cpp
#include "../ProvidedGUI/ChessFrontend.hpp"
```

אם מבנה התיקיות שלכם שונה, התאימו את נתיב ה-`include`.

יצירת האובייקט:

```cpp
ChessFrontend gui;
```

יצירת האובייקט פותחת את חלון המשחק.

---

# 3. לולאת GUI בסיסית

```cpp
ChessFrontend gui;

while (gui.isOpen()) {
    ChessFrontend::UiEvent event = gui.pollEvent();

    // Handle the event here.

    gui.render();
}
```

`pollEvent()` מדווח לקוד שלכם על פעולת המשתמש.

`render()` מצייר את המצב הנוכחי של ה-GUI.

---

# 4. הצגת הלוח

ה-GUI מקבל:

```cpp
std::array<char, 64>
```

הייצוג הוא:

```text
K = White King
Q = White Queen
R = White Rook
B = White Bishop
N = White Knight
P = White Pawn

k = Black King
q = Black Queen
r = Black Rook
b = Black Bishop
n = Black Knight
p = Black Pawn

. או # = משבצת ריקה
```

סדר המשבצות:

```text
index 0  = a8
index 7  = h8

index 56 = a1
index 63 = h1
```

כדי לעדכן את הלוח:

```cpp
gui.setBoard(board);
```

ה-GUI רק מציג את המערך שקיבל.

---

# 5. קבלת מהלך מהמשתמש

כאשר המשתמש לוחץ על משבצת מקור ולאחר מכן על משבצת יעד, מתקבל:

```cpp
event.type ==
    ChessFrontend::UiEventType::MoveRequested
```

המשבצות נמצאות ב:

```cpp
event.from.row
event.from.col

event.to.row
event.to.col
```

לדוגמה:

```cpp
if (event.type ==
    ChessFrontend::UiEventType::MoveRequested) {

    int fromRow = event.from.row;
    int fromCol = event.from.col;

    int toRow = event.to.row;
    int toCol = event.to.col;

    // מכאן הקוד שלכם מחליט מה לעשות.
}
```

**ה-GUI אינו בודק אם המהלך חוקי.**


---

# 6. הפעלה וכיבוי של קלט הלוח

ניתן לחסום זמנית לחיצות על משבצות הלוח:

```cpp
gui.setBoardInputEnabled(false);
```

ולהחזיר את הקלט:

```cpp
gui.setBoardInputEnabled(true);
```

הפעולה אינה משנה את הלוח המוצג. היא רק מונעת יצירה של `MoveRequested` מלחיצות על הלוח.


הקוד שלכם הוא שאחראי להחליט מתי להפעיל או לכבות את הקלט.

---

# 7. הצגת שמות השחקנים והתור הנוכחי

ניתן להציג את שמות שני השחקנים באמצעות:

```cpp
gui.setPlayerNames("Alice", "Bob");
```

הפרמטר הראשון הוא שם השחקן הלבן והפרמטר השני הוא שם השחקן השחור.
השם של השחקן השחור יוצג מעל הלוח, והשם של השחקן הלבן יוצג מתחת ללוח.

כדי להציג את התור הנוכחי:

```cpp
gui.setTurnText("White to move");
```

או:

```cpp
gui.setTurnText("Black to move");
```

הקוד שלכם אחראי לדעת מי השחקן הנוכחי.

---

# 8. הצגת מצב המשחק

לדוגמה, אם הקוד שלכם זיהה Check:

```cpp
gui.setStatusText("Check");
```

כאשר אין הודעה מיוחדת:

```cpp
gui.setStatusText("");
```

ה-GUI אינו מזהה Check בעצמו.

---

# 9. Promotion

כאשר מנוע השחמט שלכם קובע שנדרש Promotion:

```cpp
gui.showPromotionDialog("White");
```

או:

```cpp
gui.showPromotionDialog("Black");
```

המשתמש יקבל אפשרות לבחור:

```text
Queen
Rook
Bishop
Knight
```

לאחר הבחירה מתקבל:

```cpp
event.type ==
    ChessFrontend::UiEventType::PromotionSelected
```

והבחירה נמצאת ב:

```cpp
event.promotion
```

הערכים האפשריים הם:

```text
Q
R
B
N
```

---

# 10. כניעה - Resign

כאשר המשתמש לוחץ על `Resign`, ה-GUI **אינו פותח חלון אישור אוטומטית**. במקום זאת מתקבל האירוע:

```cpp
event.type ==
    ChessFrontend::UiEventType::ResignRequested
```

מכאן הקוד שלכם מחליט כיצד לטפל בבקשה. אם אתם רוצים להציג למשתמש חלון אישור, קראו ל:

```cpp
gui.showResignConfirmDialog();
```

יופיע חלון:

```text
Are you sure you want to resign?

Confirm     No
```

אם המשתמש בוחר `Confirm`, מתקבל:

```cpp
event.type ==
    ChessFrontend::UiEventType::ResignConfirmed
```

אם המשתמש בוחר `No`, מתקבל:

```cpp
event.type ==
    ChessFrontend::UiEventType::ResignCancelled
```



ה-GUI אינו קובע בעצמו מי ניצח ואינו מבצע תקשורת רשת.

---

# 11. הצעת תיקו - Offer Draw

כאשר המשתמש לוחץ על `Offer Draw`, מתקבל:

```cpp
event.type ==
    ChessFrontend::UiEventType::DrawOfferRequested
```

מכאן הקוד שלכם מחליט כיצד להעביר את ההצעה לשחקן השני.



ה-GUI אינו שולח הודעות רשת בעצמו.

## הסתרת הכפתור

במצב שבו אין אפשרות להציע תיקו, למשל במשחק `Player vs Bot`, ניתן להסתיר את הכפתור:

```cpp
gui.setOfferDrawVisible(false);
```

כדי להציג אותו מחדש:

```cpp
gui.setOfferDrawVisible(true);
```

כאשר הכפתור מוסתר, לחיצה באזור שבו הוא נמצא לא תיצור `DrawOfferRequested`.

---

# 12. הצגת הצעת תיקו שהתקבלה

כאשר הקוד שלכם מקבל הודעה שלפיה היריב הציע תיקו:

```cpp
gui.showDrawOfferDialog(
    "Opponent offers a draw."
);
```

יופיע חלון עם:

```text
Accept
Decline
```

אם המשתמש אישר:

```cpp
event.type ==
    ChessFrontend::UiEventType::DrawAccepted
```

אם המשתמש דחה:

```cpp
event.type ==
    ChessFrontend::UiEventType::DrawDeclined
```

הקוד שלכם אחראי להעביר את התשובה לצד השני במידת הצורך.

---

# 13. סיום משחק

כאשר הקוד שלכם קובע שהמשחק הסתיים, ניתן להציג מסך סיכום.

ניצחון במט:

```cpp
gui.showGameOver(
    "White wins!",
    "Checkmate"
);
```

ניצחון בעקבות כניעה:

```cpp
gui.showGameOver(
    "Black wins!",
    "White resigned"
);
```

תיקו בהסכמה:

```cpp
gui.showGameOver(
    "Draw",
    "Draw agreed"
);
```

Stalemate:

```cpp
gui.showGameOver(
    "Draw",
    "Stalemate"
);
```

ה-GUI אינו קובע מתי המשחק הסתיים. הוא רק מציג את המידע שהקוד שלכם מספק לו.

---

# 14. סגירת החלון

כאשר המשתמש סוגר את חלון המשחק:

```cpp
event.type ==
    ChessFrontend::UiEventType::CloseRequested
```

עליכם להחליט כיצד המערכת שלכם מגיבה לכך.


---

# 15. סיכום ה-API המרכזי

```cpp
ChessFrontend gui;

gui.setBoard(board);
gui.setPlayerNames("White Player", "Black Player");
gui.setTurnText("White to move");
gui.setStatusText("Check");

gui.setBoardInputEnabled(true);
gui.setOfferDrawVisible(true);

gui.showPromotionDialog("White");
gui.showResignConfirmDialog();
gui.showDrawOfferDialog("Opponent offers a draw.");
gui.showGameOver("White wins!", "Checkmate");

ChessFrontend::UiEvent event = gui.pollEvent();
gui.render();
```

אירועים אפשריים:

```text
MoveRequested
ResignRequested
ResignConfirmed
ResignCancelled
PromotionSelected
DrawOfferRequested
DrawAccepted
DrawDeclined
CloseRequested
```

---

# 16. קומפילציה

הדוגמה מיועדת ל-`C++17` עם `SFML 3` בסביבת `MSYS2 UCRT64 / MinGW`.

דוגמה עבור מבנה התיקיות המופיע בתחילת המסמך:

```bash
g++ -std=c++17 ^
    client/main.cpp ^
    ProvidedGUI/ChessFrontend.cpp ^
    -IProvidedGUI ^
    -o client.exe ^
    -lsfml-graphics ^
    -lsfml-window ^
    -lsfml-system ^
    -lsfml-network
```

ניתן כמובן להוסיף לפקודה את שאר קובצי ה-`.cpp` של הפרויקט שלכם.

ה-Server אינו זקוק ל-SFML Graphics אם הוא אינו מציג GUI.

---
