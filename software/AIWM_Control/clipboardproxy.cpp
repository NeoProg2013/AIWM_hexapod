#include "clipboardproxy.h"
#include <QGuiApplication>
#include <QClipboard>

ClipboardProxy::ClipboardProxy(QObject *parent) : QObject(parent) {

}

void ClipboardProxy::insertText(QVariant text) {
	QGuiApplication::clipboard()->setText(text.toString());
}
