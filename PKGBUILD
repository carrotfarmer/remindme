pkgname=remindme
pkgver=1.0.0
pkgrel=1
pkgdesc="Lightweight system reminders for the command line"
arch=('x86_64')
url="https://github.com/carrotfarmer/remindme"
license=("MIT")
depends=("libnotify")
source=("$pkgname-$pkgver.tar.gz::https://github.com/carrotfarmer/remindme/archive/v$pkgver.tar.gz")
sha256sums=('your-checksum-here')

build() {
    cd "$srcdir/$pkgname-$pkgver"
    make
}

package() {
    install -Dm755 "$srcdir/$pkgname-$pkgver/remindme" "$pkgdir/usr/local/bin/remindme"
    install -Dm755 "$srcdir/$pkgname-$pkgver/remindd" "$pkgdir/usr/local/bin/remindd"
    install -Dm644 "$srcdir/$pkgname-$pkgver/systemd/remindd.service" "$pkgdir/usr/lib/systemd/system/remindd.service"
}
