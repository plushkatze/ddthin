# Maintainer: plushkatze <plushkatze@users.noreply.github.com>
pkgname=ddthin
pkgver=0.0.4
pkgrel=1
pkgdesc="Tool to copy thin/sparse volumes/files"
arch=('x86_64')
url="https://github.com/plushkatze/ddthin"
license=('GPL')
depends=(glibc)
source=("$pkgname-$pkgver.tar.gz")
md5sums=(SKIP)

build() {
	cd "$pkgname-$pkgver"
	make
}

package() {
	cd "$pkgname-$pkgver"
	make DESTDIR="$pkgdir/" PREFIX="/usr" install
}
