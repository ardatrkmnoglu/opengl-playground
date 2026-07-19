#pragma once

// API misuse tests
void rTest_invalidEnum() ;
void rTest_invalidValue() ;

// state machine robustness
void rTest_stateRecovr() ;

// resource management robustness
void rTest_outOfMemory() ;

// buffer and memory safety
void rTest_nullPtr() ;
void rTest_oobDraw() ;

// shader robustness
void rTest_shaderCompilerError() ;
void rTest_invalidPrecision() ;

// draw pipeline robustness
void rTest_drawWOProgram() ;
void rTest_missingAttrib() ;

// limit and capability tests
void rTest_maxTextureLimit() ;

// error handling robustness
void rTest_errorFlood();

// degenerate geometry/number handling
void rTest_NaNVertices();

