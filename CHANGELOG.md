# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [2.0.0] - 2025-03-15
### Added
- Commodities library support for hotkey-driven window management
- Comprehensive error handling for mathematical operations
- Amiga UI Style Guide compliant keyboard shortcuts
- Debug logging system with file/line tracking
- Stack overflow/underflow protection
- Memory bounds checking for all operations

### Changed
- Updated to full AmigaOS 3.x UI compliance
- Revised memory management to prevent leaks
- Improved font-sensitive layout using system metrics
- Enhanced clipboard handling with proper IFF cleanup
- Optimized 68000 FPU compatibility

### Fixed
- Resource cleanup in all exit paths
- Division by zero protection
- Factorial input validation (0-20 for integer, 0-170 for double)
- Combination/permutation validation
- Window state persistence issues
- Gadget cleanup in memory selector

### Removed
- Deprecated UI elements not compliant with Amiga HIG
- Unused equation solver and currency conversion code
- Redundant preferences file handling

## [1.2.0] - 2024-11-30
### Added
- Initial public release
- Basic scientific functions
- Memory register support
- Workbench/CLI dual launch capability
- Tape logging feature

### Changed
- Migrated from Intuition to GadTools UI
- Improved number formatting for different bases

## [1.0.0] - 1996-01-01
### Added
- Initial release by Tim Ocock
- Core calculator functionality
- Basic arithmetic operations
- Trigonometric functions
- Menu system implementation 