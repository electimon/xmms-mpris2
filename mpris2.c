#include <gio/gio.h>
#include <xmms/plugin.h>
#include <xmms/util.h>
#include <xmms/xmmsctrl.h>
#include <xmms/configfile.h>
#include "mpris-object.h"

#define _(string) (string)

#define MPRIS_BUS_NAME "org.mpris.MediaPlayer2.xmms"
#define MPRIS_OBJECT   "/org/mpris/MediaPlayer2"

#define VERSION "0.1"

static GeneralPlugin gpi;

static MprisMediaPlayer2Player *mpris_player_interface = NULL;
static int owner_id;

static gboolean on_handle_next (
    MprisMediaPlayer2Player *object,
    GDBusMethodInvocation *invocation)
{
    xmms_remote_playlist_next(gpi.xmms_session);

    mpris_media_player2_player_complete_next(object, invocation);

    return TRUE;
}

static gboolean on_handle_previous (
    MprisMediaPlayer2Player *object,
    GDBusMethodInvocation *invocation)
{
    xmms_remote_playlist_prev(gpi.xmms_session);

    mpris_media_player2_player_complete_previous(object, invocation);

    return TRUE;
}

static gboolean on_handle_pause (
    MprisMediaPlayer2Player *object,
    GDBusMethodInvocation *invocation)
{
    xmms_remote_pause(gpi.xmms_session);

    mpris_media_player2_player_complete_pause(object, invocation);

    return TRUE;
}

static gboolean on_handle_playpause(
    MprisMediaPlayer2Player *object,
    GDBusMethodInvocation *invocation)
{
    if (xmms_remote_is_playing(gpi.xmms_session)) {
        xmms_remote_pause(gpi.xmms_session);
    } else {
        xmms_remote_play(gpi.xmms_session);
    }

    mpris_media_player2_player_complete_play_pause(object, invocation);

    return TRUE;
}

static gboolean on_handle_stop(
    MprisMediaPlayer2Player *object,
    GDBusMethodInvocation *invocation)
{
    xmms_remote_stop(gpi.xmms_session);

    mpris_media_player2_player_complete_stop(object, invocation);

    return TRUE;
}

static gboolean on_handle_play(
    MprisMediaPlayer2Player *object,
    GDBusMethodInvocation *invocation)
{
    xmms_remote_play(gpi.xmms_session);

    mpris_media_player2_player_complete_play(object, invocation);

    return TRUE;
}

static void on_bus_acquired (GDBusConnection *connection,
        const gchar     *name,
        gpointer         user_data)
{
    mpris_player_interface = mpris_media_player2_player_skeleton_new ();
    g_signal_connect (mpris_player_interface,
            "handle-next",
            G_CALLBACK (on_handle_next),
            NULL);
    g_signal_connect (mpris_player_interface,
            "handle-previous",
            G_CALLBACK (on_handle_previous),
            NULL);
    g_signal_connect (mpris_player_interface,
            "handle-pause",
            G_CALLBACK (on_handle_pause),
            NULL);
    g_signal_connect (mpris_player_interface,
            "handle-play-pause",
            G_CALLBACK (on_handle_playpause),
            NULL);
    g_signal_connect (mpris_player_interface,
            "handle-stop",
            G_CALLBACK (on_handle_stop),
            NULL);
    g_signal_connect (mpris_player_interface,
            "handle-play",
            G_CALLBACK (on_handle_play),
            NULL);
    mpris_media_player2_player_set_can_go_next(mpris_player_interface, TRUE);
    mpris_media_player2_player_set_can_go_previous(mpris_player_interface, TRUE);
    mpris_media_player2_player_set_can_play(mpris_player_interface, TRUE);
    mpris_media_player2_player_set_can_pause(mpris_player_interface, TRUE);
    mpris_media_player2_player_set_can_control(mpris_player_interface, TRUE);
    GError *error = NULL;
    if( !g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (mpris_player_interface),
            connection,
            MPRIS_OBJECT,
            &error))
    {
        g_warning("g_dbus_interface_skeleton_export failed: %s", error->message);
    }
    
}

static void on_name_acquired (GDBusConnection *connection,
        const gchar     *name,
        gpointer         user_data)
{
    g_print ("Acquired the name %s on the session bus\n", name);
}

static void on_name_lost (GDBusConnection *connection,
        const gchar     *name,
        gpointer         user_data)
{
    g_print ("Lost the name %s on the session bus\n", name);
}

int mpris_init()
{
    owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
            MPRIS_BUS_NAME,
            G_BUS_NAME_OWNER_FLAGS_NONE,
            on_bus_acquired,
            on_name_acquired,
            on_name_lost,
            NULL,
            NULL);

    //loop = g_main_loop_new (NULL, FALSE);
    //g_main_loop_run (loop);

    //g_bus_unown_name (owner_id);
    return 0;
}

void mpris_handle_event()
{
    GMainContext * main_context = g_main_context_default();
    while(g_main_context_iteration(main_context, FALSE));
}

void mpris_quit()
{
}

static void plugin_init() {
    mpris_init();
}

static void plugin_cleanup() {
    g_bus_unown_name (owner_id);
}

/* plugin_about(): called when the "about" button in the plugins
 * configuration is pressed.  Typical behavior here is to present
 * a dialog explaining what the plugin does.
 */
static void plugin_about()
{
        static GtkWidget *about;
        gchar *s;

        if (about != NULL) {
                gdk_window_raise(about->window);
                return;
        }

        const gchar *s1 = _("XF86Audio Keys Control Plugin");
        const gchar *s2 = _(
                "This plugin enables you to use your damn multimedia keys\n"
                "In 2024, I just really need to be able to quickly skip tracks man\n"
                "But no XF86Audio keys are already bound to playerctl\n"
                "So i'll just suffer and write this plugin instead, Go be gay and do crime\n"
                "Written by Electimon, portions of code by zhanglei (zhangleibruce@gmail.com)\n"
                "Other portions taken from xmms-xf86audio project\n"
                "Licensed under GPL-2.0. Enjoy.");
        s = g_strdup_printf("%s %s\n\n%s\n",s1, VERSION, s2);
        about = xmms_show_message(
                        _("About xmms-mpris2 plugin"),
                        s, "OK", 1, NULL, NULL);
        gtk_signal_connect(GTK_OBJECT(about), "destroy",
                        GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about);

}

static void plugin_configure()
{
    // There is no configure, enjoy!
    return;
}

/* gst_gplugin_info(): General plugin query function.  Oddly, XMMS does
 * not supply a pointer to a GeneralPlugin structure, so we keep a static
 * one around.
 */
GeneralPlugin *get_gplugin_info()
{
        gpi.description = _("MPRIS2 playback control");
        gpi.xmms_session = -1;
        gpi.init = plugin_init;
        gpi.about = plugin_about;
        gpi.configure = plugin_configure;
        gpi.cleanup = plugin_cleanup;
        return &gpi;
}
