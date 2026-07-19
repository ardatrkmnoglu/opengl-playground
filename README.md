# [TR] opengl-playground
OpenGL için rastgele projeler ve robustness/performans testleri.

## Test Oluşturma ve Yürütme
Bütün testler `tests/` klasörünün içerisinde yer alıyor. Bu klasörün içerisindeyken mevcut testler şu şekilde çalıştırılabilir:
```bash
# Sadece derlemek için
make

# Testleri çalıştırmak için
make run
```

Ana testlerin yanı sıra önceden alıştırma için yazılmış karışık testler `rtests.c` kaynak dosyasının içerisinde bulunmaktadır. Testler derlenirken bunlar da dahil edilebilir:
```bash
# RUN_EXTESTS makrosunu tanımlar ve rtests.c içerisindeki
# karışık testleri de programa dahil eder
make CPPFLAGS="-DRUN_EXTESTS"
```

# [EN] opengl-playground
Random projects and robustness/performance tests in OpenGL as an introductory step.

## Creating & Running Tests
[under construction]

