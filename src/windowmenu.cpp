/*COPYRIGHT_HEADER

This file is a part of nm-tray.

Copyright (c)
    2016~now Palo Kisa <palo.kisa@gmail.com>

nm-tray is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

COPYRIGHT_HEADER*/

#include "windowmenu.h"
#include "nmmodel.h"
#include "nmproxy.h"
#include "menuview.h"

#include <QWidgetAction>
#include <functional>

class WindowMenuPrivate
{
public:
    NmModel * mNmModel;

    QScopedPointer<NmProxy> mWirelessModel;
    QWidgetAction * mWirelessAction;

    QScopedPointer<NmProxy> mActiveModel;
    QWidgetAction * mActiveAction;

    template <typename F>
    void onActivated(QModelIndex const & index
            , QAbstractItemModel const * topParent
            , F const & functor);
};

template <typename F>
void WindowMenuPrivate::onActivated(QModelIndex const & index
        , QAbstractItemModel const * topParent
        , F const & functor)
{
    QModelIndex i = index;
    for (QAbstractProxyModel const * proxy = qobject_cast<QAbstractProxyModel const *>(index.model())
            ; nullptr != proxy && topParent != proxy
            ; proxy = qobject_cast<QAbstractProxyModel const *>(proxy->sourceModel())
            )
    {
        i = proxy->mapToSource(i);
    }
    functor(i);
}



WindowMenu::WindowMenu(NmModel * nmModel, QWidget * parent /*= nullptr*/)
    : QMenu{parent}
    , d_ptr{new WindowMenuPrivate}
{
    Q_D(WindowMenu);
    d->mNmModel = nmModel;

    //active proxy & widgets
    d->mActiveModel.reset(new NmProxy);
    d->mActiveModel->setNmModel(d->mNmModel, NmModel::ActiveConnectionType);
    MenuView * active_view = new MenuView{d->mActiveModel.data()};
    connect(active_view, &QAbstractItemView::activated, [this, d, active_view] (const QModelIndex & index) {
        d->onActivated(index, d->mActiveModel.data(), std::bind(&NmProxy::deactivateConnection, d->mActiveModel.data(), std::placeholders::_1));
        close();
    });

    d->mActiveAction = new QWidgetAction{this};
    d->mActiveAction->setDefaultWidget(active_view);

    //wireless proxy & widgets
    d->mWirelessModel.reset(new NmProxy);
    d->mWirelessModel->setNmModel(d->mNmModel, NmModel::WifiNetworkType);
    MenuView * wifi_view = new MenuView{d->mWirelessModel.data()};
    connect(wifi_view, &QAbstractItemView::activated, [this, d, wifi_view] (const QModelIndex & index) {
        d->onActivated(index, d->mWirelessModel.data(), std::bind(&NmProxy::activateConnection, d->mWirelessModel.data(), std::placeholders::_1));
        close();
    });

    d->mWirelessAction = new QWidgetAction{this};
    d->mWirelessAction->setDefaultWidget(wifi_view);

    addSeparator();
    addSection(tr("Active connection(s)"));
    addAction(d->mActiveAction);
    addSeparator();
    addSection(tr("Wi-Fi network(s)"));
    addAction(d->mWirelessAction);
}

WindowMenu::~WindowMenu()
{
}
