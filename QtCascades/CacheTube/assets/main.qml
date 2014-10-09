import bb.cascades 1.0
import bb.system 1.0
import YTVideoManagement 1.0

TabbedPane {
    id: tabbedPane
    
    Menu.definition: MenuDefinition {
        settingsAction: SettingsActionItem {
            onTriggered: {
                var format = AppSettings.preferredVideoFormat;
                
                if (format === 22) {
                    preferredVideoFormatDropDown.selectedIndex = 0;
                } else if (format === 18) {
                    preferredVideoFormatDropDown.selectedIndex = 1;
                } else {
                    preferredVideoFormatDropDown.selectedIndex = 0;
                }
                
                autoRepeatPlaybackToggleButton.checked = AppSettings.autoRepeatPlayback;
                
                settingsSheet.open();
            }
            
            attachedObjects: [
                Sheet {
                    id: settingsSheet
                    
                    Page {
                        id: settingsPage
                        
                        titleBar: TitleBar {
                            title: qsTr("Settings")
                            
                            acceptAction: ActionItem {
                                title: qsTr("OK")
                                
                                onTriggered: {
                                    var format = AppSettings.preferredVideoFormat;
                                    
                                    if (preferredVideoFormatDropDown.selectedIndex === 0) {
                                        format = 22;
                                    } else if (preferredVideoFormatDropDown.selectedIndex === 1) {
                                        format = 18;
                                    }
                                    
                                    AppSettings.preferredVideoFormat = format;
                                    AppSettings.autoRepeatPlayback   = autoRepeatPlaybackToggleButton.checked;
                                    
                                    YTVideoManager.setPreferredVideoFormat(format);
                                    
                                    settingsSheet.close();
                                }
                            }
                            
                            dismissAction: ActionItem {
                                title: qsTr("Cancel")
                                
                                onTriggered: {
                                    settingsSheet.close();
                                }
                            }
                        }
                        
                        Container {
                            background: Color.White
                            
                            ScrollView {
                                scrollViewProperties {
                                    scrollMode: ScrollMode.Vertical
                                }
                                
                                Container {
                                    background:   Color.Transparent
                                    leftPadding:  12
                                    rightPadding: 12
                                    
                                    layout: StackLayout {
                                    }
                                    
                                    Divider {
                                    }
                                    
                                    DropDown {
                                        id:    preferredVideoFormatDropDown
                                        title: qsTr("Preferred Video Format")
                                        
                                        Option {
                                            text: qsTr("720p H.264 MP4")
                                        }
                                        
                                        Option {
                                            text: qsTr("360p H.264 MP4")
                                        }
                                    }
                                    
                                    Divider {
                                    }
                                    
                                    Container {
                                        background: Color.Transparent
                                        
                                        layout: StackLayout {
                                            orientation: LayoutOrientation.LeftToRight
                                        }
                                        
                                        Label {
                                            verticalAlignment:  VerticalAlignment.Center
                                            multiline:          true
                                            textStyle.color:    Color.Black
                                            textStyle.fontSize: FontSize.Medium
                                            text:               qsTr("Auto Repeat Playback")
                                            
                                            layoutProperties: StackLayoutProperties {
                                                spaceQuota: 1
                                            }
                                        }
                                        
                                        ToggleButton {
                                            id: autoRepeatPlaybackToggleButton
                                            
                                            layoutProperties: StackLayoutProperties {
                                                spaceQuota: -1
                                            }
                                        }
                                    }
                                    
                                    Divider {
                                    }
                                }
                            }
                        }
                    }
                }
            ]
        }
        
        helpAction: HelpActionItem {
            onTriggered: {
                helpWebView.url = qsTr("local:///assets/doc/help.html");
                
                helpSheet.open();
            }
            
            attachedObjects: [
                Sheet {
                    id: helpSheet
                    
                    Page {
                        titleBar: TitleBar {
                            title: qsTr("Help")
                            
                            dismissAction: ActionItem {
                                title: qsTr("Close")
                                
                                onTriggered: {
                                    helpSheet.close();
                                }
                            }
                        }
                        
                        actions: [
                            ActionItem {
                                title:               qsTr("Review App")
                                imageSource:         "images/review.png"
                                ActionBar.placement: ActionBarPlacement.OnBar
                                
                                onTriggered: {
                                    appWorldInvocation.trigger("bb.action.OPEN");
                                }
                                
                                attachedObjects: [
                                    Invocation {
                                        id: appWorldInvocation
                                        
                                        query: InvokeQuery {
                                            mimeType: "application/x-bb-appworld"
                                            uri:      "appworld://content/42093887"
                                        }
                                    }
                                ]
                            }
                        ]
                        
                        ScrollView {
                            scrollViewProperties {
                                scrollMode:         ScrollMode.Both
                                pinchToZoomEnabled: true
                                minContentScale:    1.0
                                maxContentScale:    4.0
                            }
                            
                            WebView {
                                id:  helpWebView
                                url: qsTr("local:///assets/doc/help.html")
                            }
                        }
                    }
                }
            ]
        }
    }
    
    Tab {
        id:          youTubeTab
        title:       qsTr("YouTube")
        imageSource: "images/youtube.png"
        
        Page {
            function onInvocationStarted() {
                var video_id = YTVideoManager.getVideoId(InvocationHelper.videoUrl);
                
                if (video_id !== "") {
                    youTubeWebView.url = "http://m.youtube.com/watch?v=" + encodeURIComponent(video_id);
                }
            }
            
            onCreationCompleted: {
                var video_id = YTVideoManager.getVideoId(InvocationHelper.videoUrl);
                
                if (video_id !== "") {
                    youTubeWebView.url = "http://m.youtube.com/watch?v=" + encodeURIComponent(video_id);
                }
                
                InvocationHelper.invocationStarted.connect(onInvocationStarted);
            }
            
            actions: [
                ActionItem {
                    title:               qsTr("Back")
                    imageSource:         "images/back.png"
                    ActionBar.placement: ActionBarPlacement.OnBar
                    
                    onTriggered: {
                        youTubeWebView.goBack();
                    }
                },
                ActionItem {
                    title:               qsTr("Home")
                    imageSource:         "images/home.png"
                    ActionBar.placement: ActionBarPlacement.OnBar
                    
                    onTriggered: {
                        youTubeWebView.url = "http://m.youtube.com/";
                    }
                },
                ActionItem {
                    title:               qsTr("Cache")
                    imageSource:         "images/cache.png"
                    ActionBar.placement: ActionBarPlacement.OnBar
                    
                    onTriggered: {
                        var video_id = YTVideoManager.getVideoId(youTubeWebView.url);
                        
                        if (video_id !== "") {
                            if (YTVideoManager.addTask(video_id)) {
                                cachingStartedToast.show();
                            } else {
                                cachingFailedToast.show();
                            }
                        }
                    }
                    
                    attachedObjects: [
                        SystemToast {
                            id:   cachingStartedToast
                            body: qsTr("Video caching started") 
                        },
                        SystemToast {
                            id:   cachingFailedToast
                            body: qsTr("Cannot cache this video. May be it is already cached?")
                        }
                    ]
                },
                ActionItem {
                    title:               qsTr("Reload")
                    imageSource:         "images/reload.png"
                    ActionBar.placement: ActionBarPlacement.InOverflow
                    
                    onTriggered: {
                        youTubeWebView.reload();
                    }
                }
            ]
            
            Container {
                background: Color.White
                
                layout: DockLayout {
                }
                
                ScrollView {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Fill
                    
                    scrollViewProperties {
                        scrollMode:         ScrollMode.Both
                        pinchToZoomEnabled: true
                        minContentScale:    1.0
                        maxContentScale:    4.0
                    }
                    
                    WebView {
                        id:  youTubeWebView
                        url: "http://m.youtube.com/"
                    }
                }
                
                ProgressIndicator {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Bottom
                    visible:             youTubeWebView.loading
                    fromValue:           0
                    toValue:             100
                    value:               youTubeWebView.loadProgress
                }
            }
        }
    }
    
    Tab {
        title:       qsTr("Cached Video")
        imageSource: "images/cache.png"
        
        Page {
            titleBar: TitleBar {
                title: qsTr("Cached Video")
            }
            
            Container {
                background: Color.White
                
                layout: DockLayout {
                }
                
                Container {
                    horizontalAlignment: HorizontalAlignment.Center
                    verticalAlignment:   VerticalAlignment.Center
                    background:          Color.Transparent
                    visible:             cacheListViewDataModel.itemsCount <= 0
                    
                    layout: StackLayout {
                    }
                    
                    ImageView {
                        horizontalAlignment: HorizontalAlignment.Center
                        imageSource:         "images/cache_empty.png"
                    }
                    
                    Label {
                        horizontalAlignment: HorizontalAlignment.Center
                        textStyle.color:     Color.Black
                        textStyle.fontSize:  FontSize.XLarge
                        text:                qsTr("No cached video")
                    }
                }
                
                ListView {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Fill
                    visible:             cacheListViewDataModel.itemsCount > 0
                    
                    property variant appSettings:    AppSettings 
                    property variant ytVideoManager: YTVideoManager
                    
                    dataModel: YTArrayDataModel {
                        id:           cacheListViewDataModel
                        videoManager: YTVideoManager
                    }
                    
                    function navigateToWebPage(url) {
                        youTubeWebView.url = url;
                        
                        tabbedPane.activeTab = youTubeTab;
                    } 
                    
                    listItemComponents: [
                        ListItemComponent {
                            type: ""
                            
                            Container {
                                id:           itemRoot
                                background:   itemSelected ? Color.create("#00A7DE") : Color.White 
                                leftPadding:  12
                                rightPadding: 12
                                
                                property bool   itemSelected: ListItem.active || ListItem.selected
                                property int    itemState:    ListItemData.state
                                property int    itemSize:     ListItemData.size
                                property int    itemDone:     ListItemData.done
                                property string itemVideoId:  ListItemData.videoId
                                property string itemTitle:    ListItemData.title
                                property string itemErrorMsg: ListItemData.errorMsg
                                
                                layout: StackLayout {
                                }
                                
                                gestureHandlers: [
                                    TapHandler {
                                        onTapped: {
                                            if (itemRoot.itemState === YTDownloadState.StateCompleted) {
                                                playerNavigationPane.push(playerPageDefinition.createObject());
                                                
                                                playerSheet.open();
                                            }
                                        }
                                        
                                        attachedObjects: [
                                            Sheet {
                                                id:          playerSheet
                                                peekEnabled: false
                                                
                                                onOpened: {
                                                    var page = playerNavigationPane.top;
                                                    
                                                    if (page.objectName === "playerPage") {
                                                        page.playVideo(itemRoot.ListItem.view.ytVideoManager.getTaskVideoURI(itemRoot.itemVideoId), itemRoot.itemTitle, itemRoot.ListItem.view.appSettings.autoRepeatPlayback);
                                                    }
                                                }
                                                
                                                NavigationPane {
                                                    id:          playerNavigationPane
                                                    peekEnabled: false
                                                    
                                                    onTopChanged: {
                                                        if (playerSheet.opened) {
                                                            if (page.objectName === "playerPage") {
                                                                page.playVideo(itemRoot.ListItem.view.ytVideoManager.getTaskVideoURI(itemRoot.itemVideoId), itemRoot.itemTitle, itemRoot.ListItem.view.appSettings.autoRepeatPlayback);
                                                            } else {
                                                                playerSheet.close();
                                                            }
                                                        }
                                                    }
                                                    
                                                    onPopTransitionEnded: {
                                                        if (page.objectName === "playerPage") {
                                                            page.disconnectSignals();
                                                        }
                                                        
                                                        page.destroy();
                                                    }
                                                    
                                                    Page {
                                                        Container {
                                                            background: Color.White
                                                        }
                                                    }
                                                }
                                                
                                                attachedObjects: [
                                                    ComponentDefinition {
                                                        id:     playerPageDefinition
                                                        source: "PlayerPage.qml"
                                                    }
                                                ]
                                            }
                                        ]
                                    }
                                ]
                                
                                contextActions: [
                                    ActionSet {
                                        ActionItem {
                                            title:       qsTr("Pause/Resume")
                                            imageSource: "images/pause.png"
                                            enabled:     itemRoot.itemState !== YTDownloadState.StateCompleted
                                            
                                            onTriggered: {
                                                if (itemRoot.itemState === YTDownloadState.StatePaused) {
                                                    itemRoot.ListItem.view.ytVideoManager.resumeTask(itemRoot.itemVideoId);
                                                } else {
                                                    itemRoot.ListItem.view.ytVideoManager.pauseTask(itemRoot.itemVideoId);
                                                }
                                            }
                                        }
                                        
                                        ActionItem {
                                            title:       qsTr("Open YouTube Page")
                                            imageSource: "images/youtube.png"
                                            
                                            onTriggered: {
                                                itemRoot.ListItem.view.navigateToWebPage(itemRoot.ListItem.view.ytVideoManager.getTaskWebURL(itemRoot.itemVideoId));
                                            }
                                        }
                                        
                                        DeleteActionItem {
                                            title:       qsTr("Delete")
                                            imageSource: "images/delete.png"
                                            
                                            onTriggered: {
                                                itemRoot.ListItem.view.ytVideoManager.delTask(itemRoot.itemVideoId);
                                                
                                                videoDeletedToast.deletedVideoId = itemRoot.itemVideoId;
                                                videoDeletedToast.show();
                                            }
                                            
                                            attachedObjects: [
                                                SystemToast {
                                                    id:             videoDeletedToast
                                                    body:           qsTr("Video deleted successfully")
                                                    button.label:   qsTr("Undo")
                                                    button.enabled: true
                                                    
                                                    property string deletedVideoId: ""
                                                    
                                                    onFinished: {
                                                        if (buttonSelection() === button) {
                                                            if (itemRoot.ListItem.view.ytVideoManager.restTask(deletedVideoId)) {
                                                                videoRestoredToast.show();
                                                            } else {
                                                                videoRestoreFailedToast.show();
                                                            }
                                                        }
                                                    }
                                                },
                                                SystemToast {
                                                    id:   videoRestoredToast
                                                    body: qsTr("Video restored successfully")
                                                },
                                                SystemToast {
                                                    id: videoRestoreFailedToast
                                                    body: qsTr("Cannot restore video")
                                                }
                                            ]
                                        }
                                    }
                                ]
                                
                                Label {
                                    multiline:            true
                                    textFormat:           TextFormat.Plain
                                    textStyle.color:      itemRoot.itemSelected ? Color.White : Color.Black
                                    textStyle.fontWeight: FontWeight.Bold
                                    textStyle.fontSize:   FontSize.Medium
                                    text:                 itemRoot.itemTitle
                                }
                                
                                ProgressIndicator {
                                    visible:   itemRoot.itemState === YTDownloadState.StateActive
                                    fromValue: 0
                                    toValue:   itemRoot.itemSize
                                    value:     itemRoot.itemDone
                                }
                                
                                Label {
                                    visible:             itemRoot.itemState !== YTDownloadState.StateActive
                                    multiline:           true
                                    textStyle.color:     itemRoot.itemState === YTDownloadState.StateError ? Color.Red : (itemRoot.itemSelected ? Color.LightGray : Color.DarkGray)
                                    textStyle.fontStyle: FontStyle.Italic
                                    textStyle.fontSize:  FontSize.Small
                                    text:                itemRoot.itemState === YTDownloadState.StateCompleted ? (itemRoot.itemSize / 1048576).toFixed(2) + " MiB" :
                                    (itemRoot.itemState === YTDownloadState.StateError     ? itemRoot.itemErrorMsg                             :
                                    (itemRoot.itemState === YTDownloadState.StateQueued    ? qsTr("QUEUED")                                    :
                                    (itemRoot.itemState === YTDownloadState.StatePaused    ? qsTr("PAUSED")                                    : "")))
                                }
                                
                                Divider {
                                }
                            }
                        }
                    ]
                }
            }
        }
    }
}
