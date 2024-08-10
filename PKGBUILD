pkgname=remindme-cli
pkgver=1.0.0
pkgrel=1
pkgdesc="Lightweight system reminders for the command line"
arch=('x86_64')
url="https://github.com/carrotfarmer/remindme"
license=("MIT")
depends=("libnotify")
source=("$pkgname-$pkgver.tar.gz::https://github.com/carrotfarmer/remindme/archive/v$pkgver.tar.gz")
sha256sums=("025728f7cc629713857aa386d7eaa295dc4507935b5a07bf7a5136ee2064ad4f")

build() {
    cd "$srcdir/remindme-$pkgver"
    make
}

package() {
    install -Dm755 "$srcdir/$pkgname-$pkgver/build/remindme" "$pkgdir/usr/local/bin/remindme"
    install -Dm755 "$srcdir/$pkgname-$pkgver/build/remindd" "$pkgdir/usr/local/bin/remindd"
    install -Dm644 "$srcdir/$pkgname-$pkgver/systemd/remindd.service" "$pkgdir/usr/lib/systemd/system/remindd.service"

    # Create /etc/.remindme and set permissions
    install -Dm644 /dev/null "$pkgdir/etc/.remindme"
    chmod 666 "$pkgdir/etc/.remindme"
}
