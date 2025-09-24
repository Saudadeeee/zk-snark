# Scripts Tiện Ích

Thư mục này chứa các script tiện ích cho việc testing và development.

## Danh sách Scripts

### `run_field_tests.sh`
- **Mục đích**: Chạy toàn bộ test suite cho field arithmetic
- **Sử dụng**: 
  ```bash
  ./scripts/run_field_tests.sh
  ```
- **Mô tả**: Script này sẽ build và chạy tất cả các test cases cho field operations, bao gồm cả 64-bit dev mode và 256-bit BN254 mode.

### `test_bn254_mode.sh`
- **Mục đích**: Test chuyển đổi và chạy test trong BN254 256-bit mode
- **Sử dụng**:
  ```bash
  ./scripts/test_bn254_mode.sh
  ```
- **Mô tả**: Script này sẽ:
  - Tự động backup config hiện tại
  - Chuyển sang USE_64BIT_DEV = false (BN254 mode)
  - Build lại project với 256-bit arithmetic
  - Chạy test_field_bn254
  - Restore lại config ban đầu

## Ghi chú

- Tất cả scripts đã được set executable permissions
- Chạy từ thư mục root của project: `./scripts/script_name.sh`
- Scripts sử dụng timeout protection để tránh infinite loops