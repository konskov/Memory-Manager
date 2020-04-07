# Memory-Manager

[//]: # (Αλφαβητικά, επώνυμο)

Επώνυμο | Όνομα | Αριθμός Μητρώου
--- | --- | ---
Σκοβολά | Κωνσταντίνα | 03115112
Κόλκας | Απόστολος | 03115142

## Οδηγίες Χρήσης 
Για την χρήση του custom memory manager κάποια εφαρμογή, ο χρήστης πρέπει να συμπεριλάβει το header file:
``` #include "mem_manager.h" ```
Επιπλέον, για τη μεταγλώττιση του κώδικα, δίνεται κατάλληλο Makefile στο οποίο πρέπει να αντικατασταθεί μόνο το όνομα του επιθυμητού πηγαίου κώδικα. Μετά την αλλαγή αυτή, το compile γίνεται με την εντολή 
```make first_fit``` ή ```make best_fit```, ανάλογα με την επιθυμητή πολιτική.
## Στόχος
Σκοπός της εργασίας ήταν να γραφεί σε C++ ένας Memory Manager για single-threaded εφαρμογές που να μπορεί να αντικαταστήσει το default Memory Manager σε όλες τις κλήσεις new/delete. Επίσης, να μπορεί να αντιμετωπίζει τον κατακερματισμό της μνήμης ακολουθώντας κάποια από τις πολιτικές που αναφέρθηκαν στο μάθημα.

* Εσωτερικός κατακερματισμός (Internal Fragmentation):
Το μέγεθος του δεσμευμένου μπλοκ είναι μεγαλύτερο από αυτό που χρειάζεται αυτός που το δεσμεύει.
* Εξωτερικός κατακερματισμός (External Fragmentation):
Συνολικά υπάρχει διαθέσιμος χώρος αλλά κανένα μπλοκ μόνο του δεν είναι αρκετά μεγάλο για να ικανοποιηθεί το αίτημα για μνήμη.

Για την αντιμετώπιση του internal fragmentation η υλοποίησή μας υποστηρίζει **block splitting**. Για την αντιμετώπιση του external fragmentation υποστηρίζει **block coalescing**. 

## Χαρακτηριστικά Συστήματος
* CPU: 
8 cores στα 2.6 GHz
* Cache: 

| L1D-L1I  |    L2     |  L3   |
|---------|:--------:|--------|
| 32 KB   | 256 KB | 6144 KB |

* RAM: 
8037900KB ή 8GB

## Σύντομη περιγραφή 
Ο memory manager που σχεδιάσαμε αντικαθιστά τις βασικές λειτουργίες δυναμικής διαχείρισης μνήμης new και delete της c++. Εσωτερικά, πραγματοποιεί ακόμα βελτιστοποιήσεις διαχείρισης του διαθέσιμου χώρου μνήμης με εφαρμογή κατάλληλων spliting και coalecing. Η πολιτική που χρησιμοποιείται (best fit ή first fit) μπορεί να επιλεγεί από τον χρήστη κατά τη μεταγλώττιση. 
Οι περισσότερες σχεδιαστικές επιλογές έχουν γίνει με κριτήριο την προγραμματιστική ευκολία σε βάρος της επίδοσης. Για αυτόν τον λόγο, επιλέξαμε να βάλουμε τα memory blocks σε μια απλά συνδεδεμένη λίστα. Συνεπώς, ο memory manager παρουσιάζει βέλτιστη συμπεριφορά όταν χρησιμοποιείται σαν stack. 

## Αναλυτική περιγραφή - Επεξήγηση βασικών συναρτήσεων
Το βασικό struct της υλοποίησης είναι το Block. Βάση αυτού, συνθέτουμε τις λίστες blocks και free blocks, οποίες χρησιμοποιούμε για να βελτιώσουμε τους χρόνους αναζήτησης.  
Για την προσπέλαση του χώρου μνήμης σχεδιάσαμε συναρτήσεις για τη μέτρηση του διαθέσιμου και του ελεύθερου χώρου, καθώς και για αναζήτηση της βέλτιστης θέσης προσθήκης, ανάλογα με την πολιτική που χρησιμοποιείται κάθε φορά. Επιπλέον, υπάρχουν συναρτήσεις που αναλαμβάνουν το spliting και coalescing, όπου είναι αυτό δυνατόν. Για βέλτιστη συμπεριφορά, αναζητούμε αν υπάρχει αυτή η δυνατότητα κάθε φορά που προσθέτουμε ή αφαιρούμε κάποιο block. Συγκεκριμένα, κάθε φορά που κάνουμε allocate ένα νέο block ελέγχουμε αν το fit είναι τέλειο, διαφορετικά επιλέγουμε να το κάνουμε split. Επιπλέον, κάθε φόρα που ελευθερώνουμε κάποιο block ελέγχουμε αν το επόμενο του είναι επίσης ελεύθερο. Αν ναι, τότε κάνουμε coalesce τα δύο blocks. 
Για να επιτύχουμε λοιπόν βέλτιστο external defragmentation, πρέπει τα blocks να απελευθερώνονται με την ανάποδη σειρά από αυτήν που δημιουργήθηκαν.
Αν η μνήμη που ζητά η εφαρμογή δεν υπάρχει σαν αντίστοιχο ελεύθερο block, τότε στέλνει αίτημα στο λειτουργικό για εκχώρηση περισσότερης μνήμης. 
Για προσθήκη νέων στοιχείων, και οι δύο διαθέσιμες πολιτικές χρησιμοποιούν γραμμική προσπέλαση του χώρου μνήμης. Στην περίπτωση της first fit παραχωρείται το πρώτο κατάλληλο διαθέσιμο block που εντοπίζει η αναζήτηση. Στην best fit κάνουμε πρώτα προσπέλαση όλης της λίστας με σκοπό την εύρεση του μικρότερου κατάλληλου block μνήμης. Ειδική περίπτωση είναι η εύρεση block μεγέθους ακριβώς ίδιο με το ζητούμενο, όπου και το δεσμεύομε άμεσα, τερματίζοντας την αναζήτηση. 
Για την ευκολία ενσωμάτωσης σε προγράμματα τρίτων, η εντολή new έχει γίνει overide με την alloc και η delete με τη free. Επιπλέον, για να είναι δυνατή η χρήση σε διαφορετικές πλατφόρμες, έχουμε φροντίσει να κάνουμε allign τα blocks να χρησιμοποιούμε κατάλληλες μεταβλητές (size_t, intptr_t).  


## Μετρήσεις 

Οι μετρήσεις αφορούν την εξής απλή εφαρμογή:
```
int main(int argc, char **argv)
{
    int* array[100];
    clock_t start, end; 
         
    for (int i = 0; i < 50; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            array[j] = new int(j);
            
        }
        for (int j = 0; j < 100; j++)
        {
            delete array[j];
        }

    }
    return 0;
}
```
Τα αποτελέσματα που δίνει το valgrind ( ``` valgrind --tool=massif ./file ```) για το memory footprint:
* Default MM

![default_memory_footprint](https://github.com/konskov/Memory-Manager/blob/master/screens_mm/mem_footprint_default.png)

* Custom MM - First Fit Policy

![ff_memory_footprint](https://github.com/konskov/Memory-Manager/blob/master/screens_mm/mem_footprint_ff.png)


* Custom MM - Best Fit Policy

![bf_memory_footprint](https://github.com/konskov/Memory-Manager/blob/master/screens_mm/mem_footprint_bf.png)

Και για τα memory accesses (```valgrind --log-file="mem_accesses_log.txt" --tool=lackey --trace-mem=yes ./drr```):

![memory_accesses](https://github.com/konskov/Memory-Manager/blob/master/screens_mm/mem_accesses.png)

Στην παραπάνω εφαρμογή δεν χρησιμοποιείται καθόλου ούτε το block coalescing ούτε το block splitting, γιατί η σειρά με την οποία γίνονται οι δεσμεύσεις και οι αποδεσμεύσεις μνήμης δεν το επιτρέπουν.
Αν στον παραπάνω κώδικα αλλάξουμε τη σειρά με την οποία απελευθερώνεται η μνήμη (δηλαδή αντικαθιστώντας τη γραμμή ```delete array[j]``` με τη γραμμή ```delete array[99-j]```, τότε η επίδοση βελτιώνεται πολύ καθώς αξιοποιούνται οι μηχανισμοί αυτοί. Ο αριθμός των προσβάσεων στη μνήμη μειώνεται πολύ και πλέον ανήκει στην ίδια τάξη μεγέθους με αυτόν του default MM. Το memory footprint δεν αλλάζει. Οι χρόνοι εκτέλεσης επίσης μειώνονται δραματικά (αν και πάλι είναι περίπου τριπλάσιοι από αυτούς του default MM).
Όσον αφορά τις προσβάσεις στη μνήμη έχουμε τώρα :

![memory_accesses](https://github.com/konskov/Memory-Manager/blob/master/screens_mm/mem_footprint_reversed.png)

Οι χρόνοι εκτέλεσης συγκριτικά είναι (Σημείωση: Έχουμε αλλάξει το μέγεθος του πίνακα από 100 σε 1000 και τις επαναλήψεις του εξωτερικού βρόχου από 50 σε 5000). 

| ``` delete array[j] ``` | ``` delete array [999-j] ``` |
| ---- | ---- |
|![execution_time](https://github.com/konskov/Memory-Manager/blob/master/screens_mm/times_slow.png) | ![execution_time](https://github.com/konskov/Memory-Manager/blob/master/screens_mm/times_reversed.png) |


Για τη μελέτη διαχείρησης των blocks χρησιμοποιήθηκε το παρακάτω πρόγραμμα:

```
struct Stuffing {
    double some_stuffing ;
    double even_more_stuffing;
    long long int stuffing_for_ages;
    long int one_to_stuff_them_all;
};


int main(int argc, char **argv)
{
    int* array[10];
    Stuffing* Some_block[10];
    
        for (int j = 0; j < 4; j++)
        {
            cout << j+1 << " new int" << endl << endl;
            array[j] = new int(j); 
        }
        
        for (int j = 0; j < 1; j++)
        {
            cout << j+1 << " new struct" << endl << endl ;
            
            Some_block[j] = new Stuffing();
        }
        
        for (int j = 0; j < 4; j++)
        {
            cout << j+1 << " delete int" << endl << endl;
            delete array[3-j];
        }

        for (int j = 0; j < 1; j++)
        {
            cout << j+1 << " new struct" << endl << endl ;
            
            Some_block[j] = new Stuffing();
        }
    return 0;
}
```

Και η έξοδος μετρήθηκε ως εξής:

```
calling mem manager constructor
1 new int

Total Memory Blocks Available
7952 : 8 , 1    
heapSize 1

Free Memory Blocks Available

freeSize 0

2 new int

Total Memory Blocks Available
7952 : 8 , 1    7992 : 8 , 1    
heapSize 2

Free Memory Blocks Available

freeSize 0

3 new int

Total Memory Blocks Available
7952 : 8 , 1    7992 : 8 , 1    8032 : 8 , 1    
heapSize 3

Free Memory Blocks Available

freeSize 0

4 new int

Total Memory Blocks Available
7952 : 8 , 1    7992 : 8 , 1    8032 : 8 , 1    8072 : 8 , 1    
heapSize 4

Free Memory Blocks Available

freeSize 0

1 new struct

Total Memory Blocks Available
7952 : 8 , 1    7992 : 8 , 1    8032 : 8 , 1    8072 : 8 , 1    8112 : 32 , 1    
heapSize 5

Free Memory Blocks Available

freeSize 0

1 delete int

Total Memory Blocks Available
7952 : 8 , 1    7992 : 8 , 1    8032 : 8 , 1    8072 : 8 , 0    8112 : 32 , 1    
heapSize 5

Free Memory Blocks Available
8072: 8,0 
freeSize 1

2 delete int

Total Memory Blocks Available
7952 : 8 , 1    7992 : 8 , 1    8032 : 48 , 0    8112 : 32 , 1    
heapSize 4

Free Memory Blocks Available
8032: 48,0 
freeSize 1

3 delete int

Total Memory Blocks Available
7952 : 8 , 1    7992 : 88 , 0    8112 : 32 , 1    
heapSize 3

Free Memory Blocks Available
7992: 88,0 
freeSize 1

4 delete int

Total Memory Blocks Available
7952 : 128 , 0    8112 : 32 , 1    
heapSize 2

Free Memory Blocks Available
7952: 128,0 
freeSize 1

1 new struct

Total Memory Blocks Available
7952 : 32 , 1    8016 : 64 , 0    8112 : 32 , 1    
heapSize 3

Free Memory Blocks Available
8016: 64,0 
freeSize 1

calling mem manager destructor
heapsize - total_remove - total_add 3-4-5
```
Μπορούμε να παρατηρήσουμε πώς γίνεται η δέσμευση και απελευθέρωση των blocks ( 0 = ελεύθερο, 1 = δεσμευμένο ), καθώς και το μέγεθος τους σε bytes. Βλέπουμε επίσης, πώς με χρήση coalesce και splitting μπορούμε να αποθηκεύσουμε το μεγαλύτερου μεγέθους struct, εκμεταλλευόμενοι ακριβώς όσο χώρο χρειαζόμαστε, αντί να αφήνουμε τα blocks που περιείχαν int αχρησιμοποίητα.



 
