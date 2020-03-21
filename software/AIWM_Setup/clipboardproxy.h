#ifndef CLIPBOARDPROXY_H
#define CLIPBOARDPROXY_H

#include <QObject>
#include <QVariant>

class ClipboardProxy : public QObject
{
	Q_OBJECT
public:
	explicit ClipboardProxy(QObject *parent = nullptr);
	Q_INVOKABLE void insertText(QVariant text);
};

#endif // CLIPBOARDPROXY_H
