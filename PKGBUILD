# Maintainer: plushkatze <plushkatze@users.noreply.github.com>
pkgname=ddthin
pkgver=0.0.4
pkgrel=1
pkgdesc="Tool to copy thin/sparse volumes/files"
arch=('x86_64')
url="https://github.com/plushkatze/ddthin"
license=('GPL')
depends=(glibc)
source=("https://github.com/plushkatze/ddthin/releases/download/$pkgver/$pkgname-$pkgver.tar.gz")
sha256sums=("6237ffdab150e541e16b78d3100da97a80a2d3472f46cb40312905f9ceff3429")

build() {
	cd "$pkgname-$pkgver"
	make
}

package() {
	cd "$pkgname-$pkgver"
	make DESTDIR="$pkgdir/" PREFIX="/usr" install
}
