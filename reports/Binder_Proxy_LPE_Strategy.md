# Binder Proxy LPE Strategy: Bypassing SELinux on Xiaomi

## Giriş
Android ekosisteminde `/dev/xiaomi-touch` ve `/dev/fortsense_fp` gibi aygıt düğümleri `system:system` izinleriyle korunur ve SELinux politikaları nedeniyle `untrusted_app` (normal uygulama) bağlamından doğrudan erişilemez. Bu engel, bir **Binder Proxy** (Vekil) kullanılarak aşılabilir.

## Proxy Mekanizması
Xiaomi, kullanıcı alanı uygulamalarının (örn: Game Turbo, SystemUI) donanım parametrelerini değiştirebilmesi için özel Binder servisleri sunar. Bu servisler `system` yetkisiyle çalışır ve ilgili kernel sürücülerini açma yetkisine sahiptir.

### Hedef Servis: `ITouchFeature`
`vendor.xiaomi.hw.touchfeature@1.0::ITouchFeature` servisi, kernel'daki `xiaomi_touch` sürücüsüyle konuşan ana arayüzdür.

1. **İletişim:** Normal bir uygulama bu servise Binder üzerinden bağlanabilir.
2. **Manipülasyon:** Servisin `setModeLongValue` fonksiyonu, kullanıcıdan bir tamsayı dizisi (vector) alır.
3. **Bypass:** Servis, aldığı bu veriyi kernel'a `SET_LONG_VALUE` IOCTL'i ile gönderir. Bu işlem sırasında servis "vekil" görevi görerek bizim adımıza SELinux ve dosya izini engellerini aşar.

## Exploit Zinciri Akışı
1. **Unprivileged App:** Binder aracılığıyla `ITouchFeature` servisini çağırır.
2. **System Service:** Bizim sağladığımız 256 birim uzunluğundaki hatalı veriyi kernel'a paslar.
3. **Kernel Driver:** `xiaomi_touch_dev_ioctl` fonksiyonunda stack taşması gerçekleşir.
4. **Execution:** Stack üzerindeki dönüş adresi ezilerek ROP zinciri başlatılır ve Root yetkisi elde edilir.

## Sonuç
Bu yöntem, Android'in en güçlü savunma mekanizmalarından biri olan SELinux'u, sistemin kendi servislerini kullanarak etkisiz hale getirir.
