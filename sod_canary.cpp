#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <string>
#include "sod_logic.hpp"

using namespace std;

// ---------------------------------------------------------
// Visual Logger (Replicates Python format_smod_log)
// ---------------------------------------------------------
void log_smod_run(const vector<Point>& path_a, const vector<Point>& path_b) {
    size_t len_a = path_a.size();
    size_t len_b = path_b.size();
    size_t n = (len_a < len_b) ? len_a : len_b;

    cout << "\n--- SIOD Detailed Uncanceled Contribution Log ---" << endl;
    cout << "Level | Side A (row, col)    | Side B (row, col)    | Canc | Unc  | Unc * level" << endl;
    cout << "---------------------------------------------------------------------------------------" << endl;

    if (n == 0) {
        cout << "\nTotal Stability Score: 0\nStatus: Empty paths\n";
        return;
    }

    // 1) Simulation phase – identical to calculate_smod_pair
    unordered_set<int> a_rows, a_cols;
    unordered_set<int> b_rows, b_cols;
    a_rows.reserve(n); a_cols.reserve(n);
    b_rows.reserve(n); b_cols.reserve(n);

    bool terminated = false;
    size_t final_level = 0;

    for (size_t level = 0; level < n; ++level) {
        final_level = level;
        const Point& pA = path_a[level];
        const Point& pB = path_b[level];

        a_rows.insert(pA.r); a_cols.insert(pA.c);
        b_rows.insert(pB.r); b_cols.insert(pB.c);

        // Termination Condition: Check if ANY level (0 to current) on either side
        // now has BOTH coordinates covered by the other side's accumulated visits.
        for (size_t check_level = 0; check_level <= level; ++check_level) {
            const Point& checkA = path_a[check_level];
            const Point& checkB = path_b[check_level];
            
            bool term_A = (b_rows.count(checkA.r) && b_cols.count(checkA.c));
            bool term_B = (a_rows.count(checkB.r) && a_cols.count(checkB.c));
            
            if (term_A || term_B) {
                terminated = true;
                break;
            }
        }
        
        if (terminated) {
            break;
        }
    }

    // 2) Scoring + logging phase – use FINAL sets (global cancellations)
    double total_score = 0.0;

    for (size_t level = 0; level <= final_level; ++level) {
        const Point& pA = path_a[level];
        const Point& pB = path_b[level];

        bool canc_Ar = (b_rows.count(pA.r) > 0);
        bool canc_Ac = (b_cols.count(pA.c) > 0);
        bool canc_Br = (a_rows.count(pB.r) > 0);
        bool canc_Bc = (a_cols.count(pB.c) > 0);

        int canc = (int)canc_Ar + (int)canc_Ac + (int)canc_Br + (int)canc_Bc;
        int unc  = 4 - canc;
        double lvl_score = static_cast<double>(level) * static_cast<double>(unc);
        total_score += lvl_score;

        // pretty printing with parentheses around canceled components
        cout << setw(5) << level << " | "
             << "(" << (canc_Ar ? "(" : " ") << pA.r << (canc_Ar ? ")" : " ") << ", "
             << (canc_Ac ? "(" : " ") << pA.c << (canc_Ac ? ")" : " ") << ")   | "
             << "(" << (canc_Br ? "(" : " ") << pB.r << (canc_Br ? ")" : " ") << ", "
             << (canc_Bc ? "(" : " ") << pB.c << (canc_Bc ? ")" : " ") << ")   | "
             << "  " << canc << "  |  " << unc << "   | "
             << unc << " * " << level << " = " << lvl_score << endl;
    }

    cout << "\nTotal Stability Score: " << total_score << endl;
    if (terminated) {
        cout << "Status: Terminated (Type 0)" << endl;
    } else {
        cout << "Status: Unterminated" << endl;
    }
}


// Generate Serpentine Path (6x6 grid, matching Python version)
vector<Point> generate_serpentine_path(int grid_size) {
    vector<Point> path;
    for (int r = 0; r < grid_size; ++r) {
        if (r % 2 == 0) {
            for (int c = 0; c < grid_size; ++c) {
                path.push_back({r, c});
            }
        } else {
            for (int c = grid_size - 1; c >= 0; --c) {
                path.push_back({r, c});
            }
        }
    }
    return path;
}

// Cyclic slice: take 'length' points along the serpentine cycle starting at start_idx
vector<Point> cyclic_slice(const vector<Point>& full_path, int start_idx, int length) {
    int N = full_path.size();
    vector<Point> result;
    for (int i = 0; i < length; ++i) {
        int idx = (start_idx + i) % N;
        result.push_back(full_path[idx]);
    }
    return result;
}

int main() {
    cout << "============================================================" << endl;
    cout << "   C++ SIOD/FinSIOD CANARY TEST (SERPENTINE)" << endl;
    cout << "============================================================" << endl;

    // Setup matching Python version: 6x6 grid, indices 5 and 10, LENGTH=11
    int grid_size = 6;
    int TIME_UNITS = 10;
    int LENGTH = TIME_UNITS + 1;  // 11 points
    
    vector<Point> full_snake = generate_serpentine_path(grid_size);
    
    int idx_A = 5;
    int idx_B = 10;
    
    // Finite "time-window" trajectories with wrap-around (periodic snake)
    vector<Point> traj_A = cyclic_slice(full_snake, idx_A, LENGTH);
    vector<Point> traj_B = cyclic_slice(full_snake, idx_B, LENGTH);
    
    // Reversed trajectories (final -> initial): Level 0 becomes the final cell
    vector<Point> traj_A_rev = traj_A;
    reverse(traj_A_rev.begin(), traj_A_rev.end());
    vector<Point> traj_B_rev = traj_B;
    reverse(traj_B_rev.begin(), traj_B_rev.end());

    // Reversibility check
    vector<Point> traj_A_check = traj_A_rev;
    reverse(traj_A_check.begin(), traj_A_check.end());
    vector<Point> traj_B_check = traj_B_rev;
    reverse(traj_B_check.begin(), traj_B_check.end());
    bool check_A = (traj_A == traj_A_check);
    bool check_B = (traj_B == traj_B_check);

    // Compute SIOD (forward) and FinSIOD (reversed)
    SmodResult res_insmod = calculate_smod_pair(traj_A, traj_B);
    SmodResult res_finsmod = calculate_smod_pair(traj_A_rev, traj_B_rev);

    // Expected values from Python canary
    // Note: FinSIOD terminates at level 3 (not level 5) because A's level 0 point (2,3)
    // becomes fully covered when B visits row 2 at level 3 (B already had col 3 from level 0)
    double expected_insmod_score = 21.0;
    double expected_finsmod_score = 15.0;  // Corrected: terminates at level 3, score = 0+3+6+6 = 15
    int expected_insmod_type = 0;  // T (Terminated)
    int expected_finsmod_type = 0;   // T (Terminated)

    cout << "\n--- FULL SERPENTINE PATH (Size " << grid_size << "x" << grid_size << ", " 
         << full_snake.size() << " cells) ---" << endl;
    cout << "Complete path (row, col) sequence:" << endl;
    for (size_t i = 0; i < full_snake.size(); ++i) {
        cout << "  Index " << setw(2) << i << ": (" << full_snake[i].r << ", " << full_snake[i].c << ")" << endl;
    }
    cout << "\nSelected cells for testing:" << endl;
    cout << "  Cell A: Index " << idx_A << " = (" << full_snake[idx_A].r << ", " << full_snake[idx_A].c << ")" << endl;
    cout << "  Cell B: Index " << idx_B << " = (" << full_snake[idx_B].r << ", " << full_snake[idx_B].c << ")" << endl;

    cout << "\n--- REVERSIBILITY VERIFICATION ---" << endl;
    cout << "Check Reverse Identity (Path A): " << (check_A ? "YES" : "NO") 
         << " (traj_A == traj_A_rev[::-1])" << endl;
    cout << "Check Reverse Identity (Path B): " << (check_B ? "YES" : "NO")
         << " (traj_B == traj_B_rev[::-1])" << endl;
    if (check_A && check_B) {
        cout << "  ✓ Reversibility verified: forward and reversed paths are exact inverses" << endl;
    } else {
        cout << "  WARNING: Reversibility check failed!" << endl;
    }

    cout << "\n--- FORWARD PATHS (SIOD inputs) ---" << endl;
    cout << "Path A forward (len=" << traj_A.size() << "): ";
    for (size_t i = 0; i < traj_A.size(); ++i) {
        cout << "(" << traj_A[i].r << "," << traj_A[i].c << ")";
        if (i < traj_A.size() - 1) cout << " ";
    }
    cout << endl;
    
    cout << "Path B forward (len=" << traj_B.size() << "): ";
    for (size_t i = 0; i < traj_B.size(); ++i) {
        cout << "(" << traj_B[i].r << "," << traj_B[i].c << ")";
        if (i < traj_B.size() - 1) cout << " ";
    }
    cout << endl;

    cout << "\n--- REVERSED PATHS (FinSIOD inputs; Level 0 is final cell) ---" << endl;
    cout << "Path A reversed (len=" << traj_A_rev.size() << "): ";
    for (size_t i = 0; i < traj_A_rev.size(); ++i) {
        cout << "(" << traj_A_rev[i].r << "," << traj_A_rev[i].c << ")";
        if (i < traj_A_rev.size() - 1) cout << " ";
    }
    cout << endl;
    
    cout << "Path B reversed (len=" << traj_B_rev.size() << "): ";
    for (size_t i = 0; i < traj_B_rev.size(); ++i) {
        cout << "(" << traj_B_rev[i].r << "," << traj_B_rev[i].c << ")";
        if (i < traj_B_rev.size() - 1) cout << " ";
    }
    cout << endl;

    cout << "\n" << string(80, '=') << endl;
    cout << "SIOD CALCULATION (Forward Paths)" << endl;
    cout << string(80, '=') << endl;
    cout << "Using forward trajectories: traj_A vs traj_B" << endl;
    cout << "This tests the SIOD logic function (calculate_smod_pair) with forward paths" << endl;
    cout << "as it will be used in the SIOD calculation in the pipeline." << endl;
    cout << "SIOD (Forward) A vs B - Result: Type " << res_insmod.type_code << endl;
    log_smod_run(traj_A, traj_B);
    
    cout << "\n" << string(80, '=') << endl;
    cout << "FinSIOD CALCULATION (Reversed Paths)" << endl;
    cout << string(80, '=') << endl;
    cout << "Using reversed trajectories: traj_A_rev vs traj_B_rev" << endl;
    cout << "This tests the SIOD logic function (calculate_smod_pair) with reversed paths" << endl;
    cout << "as it will be used in the FinSIOD calculation in the pipeline." << endl;
    cout << "FinSIOD (Reversed) A_rev vs B_rev - Result: Type " << res_finsmod.type_code << endl;
    log_smod_run(traj_A_rev, traj_B_rev);

    // Validation
    bool insmod_score_ok = (abs(res_insmod.score - expected_insmod_score) < 1e-6);
    bool insmod_type_ok = (res_insmod.type_code == expected_insmod_type);
    bool finsmod_score_ok = (abs(res_finsmod.score - expected_finsmod_score) < 1e-6);
    bool finsmod_type_ok = (res_finsmod.type_code == expected_finsmod_type);

    cout << "\n" << string(80, '=') << endl;
    cout << "VALIDATION RESULTS" << endl;
    cout << string(80, '=') << endl;
    cout << "This validates that:" << endl;
    cout << "  1. The SIOD logic function (calculate_smod_pair) works correctly" << endl;
    cout << "  2. Both SIOD (forward) and FinSIOD (reversed) produce expected results" << endl;
    cout << "  3. These are the SAME functions used in the pipeline calculations" << endl;
    cout << string(80, '=') << endl;
    cout << "\nINSMOD Validation (Forward Paths):" << endl;
    cout << "  - Score Match: " << (insmod_score_ok ? "YES" : "NO") 
         << " (Expected: " << expected_insmod_score << ", Got: " << res_insmod.score << ")" << endl;
    cout << "  - Type Match: " << (insmod_type_ok ? "YES" : "NO")
         << " (Expected: " << expected_insmod_type << ", Got: " << res_insmod.type_code << ")" << endl;
    if (insmod_score_ok && insmod_type_ok) {
        cout << "  ✓ SIOD calculation verified: SIOD logic works correctly with forward paths" << endl;
    }
    cout << "\nFINSMOD Validation (Reversed Paths):" << endl;
    cout << "  - Score Match: " << (finsmod_score_ok ? "YES" : "NO")
         << " (Expected: " << expected_finsmod_score << ", Got: " << res_finsmod.score << ")" << endl;
    cout << "  - Type Match: " << (finsmod_type_ok ? "YES" : "NO")
         << " (Expected: " << expected_finsmod_type << ", Got: " << res_finsmod.type_code << ")" << endl;
    if (finsmod_score_ok && finsmod_type_ok) {
        cout << "  ✓ FinSIOD calculation verified: SIOD logic works correctly with reversed paths" << endl;
    }

    if (insmod_score_ok && insmod_type_ok && finsmod_score_ok && finsmod_type_ok) {
        cout << "\n[SUCCESS] C++ SIOD/FinSIOD Canary Passed." << endl;
        return 0;
    } else {
        cout << "\n[FAILURE] C++ SIOD/FinSIOD Canary Failed!" << endl;
        return 1;
    }
}

