pkgname=remindme
pkgver=1.0
pkgrel=1
pkgdesc="A simple reminder CLI and daemon"
arch=('x86_64')
url="https://github.com/yourusername/your-project"
license=('MIT')
depends=('some-dependency')
source=("$pkgname-$pkgver.tar.gz::https://github.com/yourusername/your-project/archive/v$pkgver.tar.gz")
sha256sums=('your-checksum-here')

build() {
    cd "$srcdir/$pkgname-$pkgver"
    make
}

package() {
    install -Dm755 "$srcdir/$pkgname-$pkgver/remindme" "$pkgdir/usr/local/bin/remindme"
    install -Dm755 "$srcdir/$pkgname-$pkgver/remindd" "$pkgdir/usr/local/bin/remindd"
    install -Dm644 "$srcdir/$pkgname-$pkgver/systemd/reminder-daemon.service" "$pkgdir/usr/lib/systemd/system/reminder-daemon.service"
}
