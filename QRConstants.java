public interface QRConstants {
        public final static int FALSE_READ_ONLY = 0;
        public final static int TRUE_READ_ONLY = 1;
        public final static int FALSE = 2;
        public final static int TRUE = 3;
        public final static String EC1_MODE = "0111";
        public final static String NUMERIC_MODE = "0001";
        public final static String ALPHANUMERIC_MODE = "0010";
        public final static String BYTE_MODE = "0100";
        public final static String KANJI_MODE = "1000";
        public final static String STRUCTURED_APPEND_MODE = "0011";
        public final static String FNC1_FIRST_POS_MODE = "0101";
        public final static String FNC1_LAST_POS_MODE = "1001";
        public final static String TERMINATOR = "0000"; // end of message
        public final static int[][] CCI_TABLE = { { 10, 9, 8, 8 }, { 12, 11, 16, 10 }, { 14, 13, 16, 12 } };
        public final static int XOR_MASK = 0b101010000010010;
        // defining the format information
        // ECL = Error Correction Level
        public final static int ECL_INDEX = 14;
        public final static int ECL_LENGTH = 2;
        public final static int MPR_INDEX = 12;
        public final static int MPR_LENGTH = 3;
        public final static int FORMAT_GX = 0b10100110111;
        public final static int VERSION_GX = 0b1111100100101;
        public final static int FORMAT_OFFSET = 10;
        public final static int PD_WIDTH = 7;
        public final static int[][] POSITION_DETECTOR = new int[PD_WIDTH][PD_WIDTH];
        public final static int[][] ALIGNMENT_PATTERN = new int[5][5];
        public final static String ALPHANUMERIC_TABLE = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
        public final static int[] PAD_BYTE_1 = { FALSE, FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE };
        public final static int[] PAD_BYTE_2 = { TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE };
        public final static int TIMING_PATTERN = 6;
        // public final static int[] ERROR_CODEWORD_COUNT_L = { 19, 34, 55, 80, 108,
        // 136, 156, 194, 232, 274, 370, 428,
        // 461, 523, 589, 647, 721, 795, 861, 932, 1006, 1094, 1174, 1276, 1370, 1468,
        // 1531, 1631, 1735,
        // 1843, 1955, 2071, 2191, 2306, 2434, 2566, 2702, 2812, 2956 };
        public final static int[] NUM_OF_ERROR_CODES_L = { 7, 10, 15, 20, 26, 36, 40, 48, 60, 72, 80, 96, 104, 120, 132,
                        144, 168, 180, 196, 224, 224, 252, 270, 300, 312, 336, 360, 390, 420, 450, 480, 510, 540, 570,
                        570, 600, 630, 660, 720, 750 };
        public final static int[] NUM_OF_ERROR_CODES_M = { 10, 16, 26, 36, 48, 64, 72, 88, 110, 130, 150, 176, 198, 216,
                        240, 280, 308, 338, 364, 416, 442, 476, 504, 560, 588, 644, 700, 728, 784, 812, 868, 924, 980,
                        1036, 1064, 1120, 1204, 1260, 1316, 1372 };
        public final static int[] NUM_OF_ERROR_CODES_Q = { 13, 22, 36, 52, 72, 96, 108, 132, 160, 192, 224, 260, 288,
                        320, 360, 408, 448, 504, 546, 600, 644, 690, 750, 810, 870, 952, 1020, 1050, 1140, 1200, 1290,
                        1350, 1440, 1530, 1590, 1680, 1770, 1860, 1950, 2040 };
        public final static int[] NUM_OF_ERROR_CODES_H = { 17, 28, 44, 64, 88, 112, 130, 156, 192, 224, 264, 308, 352,
                        384, 432, 480, 532, 588, 650, 700, 750, 816, 900, 960, 1050, 1110, 1200, 1260, 1350, 1440, 1530,
                        1620, 1710, 1800, 1890, 1980, 2100, 2220, 2310, 2430 };
        public final static int[] BLOCK_COUNT_L = { 1, 1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 4, 6, 6, 6, 6, 7, 8, 8, 9, 9,
                        10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25 };
        public final static int[] BLOCK_COUNT_M = { 1, 1, 1, 2, 2, 4, 4, 4, 5, 5, 5, 8, 9, 9, 10, 10, 11, 13, 14, 16,
                        17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49 };
        public final static int[] BLOCK_COUNT_Q = { 1, 1, 2, 2, 4, 4, 6, 6, 8, 8, 8, 10, 12, 16, 12, 17, 16, 18, 21, 20,
                        23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68 };
        public final static int[] BLOCK_COUNT_H = { 1, 1, 2, 4, 4, 4, 8, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25,
                        25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81 };
        public final static int[][] ERROR_CODES = { NUM_OF_ERROR_CODES_L, NUM_OF_ERROR_CODES_M, NUM_OF_ERROR_CODES_Q,
                        NUM_OF_ERROR_CODES_H };
        public final static int[][] BLOCK_COUNT = { BLOCK_COUNT_L, BLOCK_COUNT_M, BLOCK_COUNT_Q, BLOCK_COUNT_H };
        public final static int[] P_VALUE[] = { { 3, 2, 1, 1 }, { 2, 0, 0, 0 }, { 1, 0, 0, 0 } };
        public final static int ECL_L = 0;
        public final static int ECL_M = 1;
        public final static int ECL_Q = 2;
        public final static int ECL_H = 3;
        public final static String TWO_THREE_SIX = "11101100";
        public final static String SEVENTEEN = "00010001";
        public final static double NATURAL_LOG_2 = Math.log(2);
        public final static int[] REMAINDER_BITS = new int[40];
        public final static int I_PLUS_J_MOD_2 = 0b000;
        public final static int I_MOD_2 = 0b001;
        public final static int J_MOD_3 = 0b010;
        public final static int I_PLUS_J_MOD_3 = 0b011;
        public final static int I_DIV_2_PLUS_J_DIV_3_MOD_2 = 0b100;
        public final static int I_J_MOD_2_PLUS_I_J_MOD_3 = 0b101;
        public final static int I_J_MOD_2_PLUS_I_J_MOD_3_MOD_2 = 0b110;
        public final static int I_PLUS_J_MOD_2_PLUS_I_J_MOD_3_MOD_2 = 0b111;
        public final static int[] MODULE_PATTERN_1 = { TRUE, FALSE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE,
                        FALSE };
        public final static int[] MODULE_PATTERN_2 = { FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, FALSE,
                        TRUE };
}